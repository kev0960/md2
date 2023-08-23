#include "argparse.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>

#include "logger.h"
#include "string_util.h"

namespace md2 {
namespace {

using json = nlohmann::json;

std::pair<std::string_view, size_t> GetOption(std::string_view arg,
                                              size_t start) {
  size_t end = arg.find(' ', start);
  std::string_view option = arg.substr(start, end - start);

  return std::make_pair(option, end);
}

// Returns the found option and the next to the end of the found option.
// If the option does not start with ", then it will treat entire option as a
// single option until it sees the space. Otherwise, it will read until it sees
// the ending ".
// If option is not found, then returns nullopt.
std::optional<std::pair<std::string_view, size_t>> GetOptionInQuote(
    std::string_view arg, size_t start) {
  if (start >= arg.size()) {
    return std::nullopt;
  }

  if (arg[start] != '"') {
    return GetOption(arg, start);
  }

  size_t end = arg.find('"', start + 1);
  if (end == std::string_view::npos) {
    return std::nullopt;
  }

  return std::make_pair(arg.substr(start, end + 1 - start), end + 1);
}

std::optional<size_t> HandleSingleOption(std::string_view arg,
                                         size_t option_detail_start,
                                         std::string* option) {
  auto [parsed_option, end] = GetOption(arg, option_detail_start);
  *option = parsed_option;

  return end;
}

std::optional<size_t> HandleSingleOption(std::string_view arg,
                                         size_t option_detail_start,
                                         int* option) {
  auto [parsed_option, end] = GetOption(arg, option_detail_start);
  *option = std::atoi(parsed_option.data());

  return end;
}

template <typename T, typename Transformer>
std::optional<size_t> HandleVectorOption(std::string_view arg,
                                         size_t option_detail_start,
                                         std::vector<T>* option,
                                         Transformer transformer) {
  auto maybe_quote_option = GetOptionInQuote(arg, option_detail_start);
  if (!maybe_quote_option) {
    return std::nullopt;
  }

  auto [quote_option, end] = maybe_quote_option.value();
  if (quote_option[0] == '"') {
    std::vector<std::string_view> paths =
        SplitStringByChar(quote_option.substr(1, quote_option.size() - 2), ',');
    for (std::string_view path : paths) {
      option->push_back(transformer(path));
    }
  } else {
    option->push_back(transformer(quote_option));
  }

  return end;
}

std::optional<size_t> ReadFromJson(std::string_view arg,
                                   size_t option_detail_start,
                                   DriverOptions* option) {
  auto [parsed_option, end] = GetOption(arg, option_detail_start);

  std::string file_name(parsed_option);
  std::ifstream in(file_name.c_str());

  json option_data;
  in >> option_data;

  if (option_data.count("input_dirs")) {
    option->input_dirs =
        option_data["input_dirs"].get<std::vector<std::string>>();
  }

  if (option_data.count("input_files")) {
    option->input_files =
        option_data["input_files"].get<std::vector<std::string>>();
  }

  if (option_data.count("book_to_dir")) {
    for (auto itr = option_data["book_to_dir"].begin();
         itr != option_data["book_to_dir"].end(); itr++) {
      option->book_file_and_dir.push_back(std::make_pair(
          (*itr)["book"].get<std::string>(), (*itr)["dir"].get<std::string>()));
    }
  }

  if (option_data.count("output_dir")) {
    option->output_dir = option_data["output_dir"].get<std::string>();
  }

  if (option_data.count("image_path")) {
    option->image_path = option_data["image_path"].get<std::string>();
  }

  if (option_data.count("json_output_dir")) {
    option->json_output_dir = option_data["json_output_dir"].get<std::string>();
  }

  if (option_data.count("clang_format_server")) {
    option->clang_format_server_path =
        option_data["clang_format_server"].get<std::string>();
  }

  if (option_data.count("use_clang_format_server")) {
    option->use_clang_format_server =
        option_data["use_clang_format_server"].get<bool>();
  }

  if (option_data.count("clang_format_server_port")) {
    option->clang_format_server_port =
        option_data["clang_format_server_port"].get<int>();
  }

  if (option_data.count("html")) {
    option->generate_html = option_data["html"].get<bool>();
  }

  if (option_data.count("latex")) {
    option->generate_latex = option_data["latex"].get<bool>();
  }

  if (option_data.count("jobs")) {
    option->num_threads = option_data["jobs"].get<int>();
  }

  if (option_data.count("auth_file_path")) {
    option->auth_file_path = option_data["auth_file_path"].get<std::string>();
  }

  if (option_data.count("update_database")) {
    option->update_database = option_data["update_database"].get<bool>();
  }

  if (option_data.count("update_database")) {
    option->use_new_schema = option_data["use_new_schema"].get<bool>();
  }

  return end;
}

auto StringViewToString = [](std::string_view s) { return std::string(s); };
std::pair<std::string, std::string> StringViewToPair(std::string_view s) {
  auto str_vec = SplitStringByCharToStringVec(s, ':');
  if (str_vec.empty()) {
    return std::make_pair("", "");
  } else if (str_vec.size() == 1) {
    return std::make_pair(str_vec[0], "");
  } else {
    return std::make_pair(str_vec[0], str_vec[1]);
  }
}

std::optional<size_t> HandleDriverOption(
    std::string_view arg, std::string_view option_name,
    std::optional<size_t> option_detail_start, DriverOptions& option) {
  if (option_name == "input_dirs") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return HandleVectorOption(arg, *option_detail_start, &option.input_dirs,
                              StringViewToString);
  } else if (option_name == "input_files") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return HandleVectorOption(arg, *option_detail_start, &option.input_files,
                              StringViewToString);
  } else if (option_name == "output_dir") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return HandleSingleOption(arg, *option_detail_start, &option.output_dir);
  } else if (option_name == "image_path") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return HandleSingleOption(arg, *option_detail_start, &option.image_path);
  } else if (option_name == "json_output_dir") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return HandleSingleOption(arg, *option_detail_start,
                              &option.json_output_dir);
  } else if (option_name == "no_html") {
    option.generate_html = false;

    return option_detail_start;
  } else if (option_name == "no_latex") {
    option.generate_latex = false;

    return option_detail_start;
  } else if (option_name == "md2_server_port") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return HandleSingleOption(arg, *option_detail_start,
                              &option.md2_server_port);
  } else if (option_name == "book_to_dir") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return HandleVectorOption(arg, *option_detail_start,
                              &option.book_file_and_dir, StringViewToPair);
  } else if (option_name == "j") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return ReadFromJson(arg, *option_detail_start, &option);
  }

  return std::nullopt;
}

DriverOptions EmitOptionFromString(std::string_view arg) {
  DriverOptions driver_option;

  size_t current = 0;
  while (true) {
    // Find the argument option that starts with "-".
    size_t option_start = arg.find('-', current);
    if (option_start == std::string_view::npos) {
      break;
    }

    // Read until it sees ' '.
    size_t option_end = arg.find(' ', option_start + 1);

    std::string_view option_name =
        arg.substr(option_start + 1, option_end - (option_start + 1));

    std::optional<size_t> option_detail_start =
        option_end != std::string_view::npos
            ? std::optional<size_t>(option_end + 1)
            : std::nullopt;

    auto maybe_end = HandleDriverOption(arg, option_name, option_detail_start,
                                        driver_option);
    if (!maybe_end) {
      break;
    }

    current = maybe_end.value();
  }

  return driver_option;
}

}  // namespace

DriverOptions ArgParse::EmitOption(int argc, char* argv[]) {
  // Let's concat entire args into single string.
  std::string arg = Join(argc, argv);

  return EmitOptionFromString(arg);
}

}  // namespace md2

#include "argparse.h"

#include <optional>

#include "logger.h"
#include "string_util.h"

namespace md2 {
namespace {

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
  } else if (option_name == "log_db") {
    option.should_log_db = true;

    return option_detail_start;
  } else if (option_name == "book_to_dir") {
    if (!option_detail_start) {
      return std::nullopt;
    }

    return HandleVectorOption(arg, *option_detail_start,
                              &option.book_file_and_dir, StringViewToPair);
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

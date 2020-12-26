#include "driver.h"

#include <fmt/color.h>
#include <fmt/core.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <optional>

#include "db.h"
#include "generators/book.h"
#include "generators/html_generator.h"
#include "generators/latex_generator.h"
#include "logger.h"
#include "parse_tree.h"
#include "parser.h"
#include "thread_pool.h"

namespace md2 {
namespace {

namespace fs = std::filesystem;

std::string ReadFileContent(const std::string& file_path) {
  std::ifstream in(file_path);

  in.seekg(0, std::ios::end);

  std::string file_content;
  file_content.reserve(in.tellg());

  in.seekg(0, std::ios::beg);
  file_content.assign(std::istreambuf_iterator<char>(in),
                      std::istreambuf_iterator<char>());

  return file_content;
}

std::string GenerateOutputPath(std::string_view file_name,
                               std::string_view output_dir,
                               std::string_view ext) {
  fs::path p(file_name);
  return StrCat(output_dir, "/", p.parent_path().c_str(), "/", p.stem().c_str(),
                ".", ext);
}

// if file_name <= 228 --> Then we should append dump_ at front.
// If the file_name is not a number then this will return nullopt.
std::optional<std::string> NextPageToActualFileName(
    const std::string& file_name) {
  int file_num;
  try {
    file_num = std::stoi(file_name);
  } catch (std::invalid_argument& e) {
    return std::nullopt;
  }

  if (file_num <= 228) {
    return StrCat("dump_", file_name, ".md");
  } else {
    return StrCat(file_name, ".md");
  }
}

}  // namespace

void Driver::Run() {
  fmt::print(fmt::fg(fmt::color::red), "Starting driver \n");
  ReadFilesInDirectory();
  BuildFileMetadataRepo();
  BuildBookFilesMap();

  DoParse();
  GenerateJSONFiles();

  if (options_.generate_latex) {
    GenerateBookMainPage();
  }

  if (options_.update_database) {
    UpdateDatabase();
  }
}

void Driver::ReadFilesInDirectory() {
  if (options_.use_clang_format_server) {
    fmt::print("Starting Clang Format Server -- \n");
    StartClangFormatServer(options_.clang_format_server_path);

    zmq_context_ = std::make_unique<zmq::context_t>(/*io_threads=*/1);
  }

  fmt::print("Start reading files -- \n");

  int total_files = 0;
  for (const auto& dir : options_.input_dirs) {
    total_files += std::distance(fs::recursive_directory_iterator(dir),
                                 fs::recursive_directory_iterator());
  }

  int num_read = 0;
  for (const auto& dir : options_.input_dirs) {
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
      const fs::path& path = entry.path();

      // Only handle md files.
      if (path.extension() != ".md") {
        continue;
      }

      file_contents_[path.filename()] = std::make_tuple(
          ReadFileContent(path), 0, fs::relative(path, dir).c_str());

      fmt::print(fmt::fg(fmt::color::green), "Reading files... [{}/{}]\n",
                 ++num_read, total_files);
      // Erase the previous line.
      fmt::print("\033[A\33[2KT\r");
    }
  }

  for (const auto& file : options_.input_files) {
    const fs::path path(file);
    file_contents_[path.filename()] =
        std::make_tuple(ReadFileContent(path), 0, path.filename());
  }
}

void Driver::BuildFileMetadataRepo() {
  for (auto& [file_name, file_data] : file_contents_) {
    auto& [content, read_pos, rel_path] = file_data;
    auto metadata =
        MetadataFactory::ParseMetadata(file_name, content, read_pos);
    if (metadata) {
      repo_.RegisterMetadata(file_name, std::move(metadata));
    }
  }
}

void Driver::DoParse() {
  fmt::print(fmt::fg(fmt::color::green), "Start parsing ... \n");
  ThreadPool pool(options_.num_threads);

  for (auto& [file_name, content_and_pos] : file_contents_) {
    auto& [file_content, pos, rel_path] = content_and_pos;
    std::string_view content(file_content.c_str() + pos);

    pool.enqueue(
        [this, content](std::string rel_path) { DoParse(content, rel_path); },
        rel_path);
  }
}

void Driver::DoParse(std::string_view content, std::string_view file_name) {
  std::string output_file_name =
      GenerateOutputPath(file_name, options_.output_dir, "html");

  fmt::print("[{}/{}] Generating [{}] to [{}] \n", ++num_parsed_,
             file_contents_.size(), file_name, output_file_name);

  Parser parser;
  const ParseTree tree = parser.GenerateParseTree(content);

  GeneratorContext context(repo_, options_.image_path,
                           options_.use_clang_format_server,
                           options_.clang_format_server_port,
                           zmq_context_.get());
  if (options_.generate_html) {
    HTMLGenerator generator(file_name, content, context, tree);
    generator.Generate();

    std::ofstream out(output_file_name);
    out << generator.ShowOutput();
  }

  if (auto itr = book_dir_to_files_.find(std::string(file_name));
      itr != book_dir_to_files_.end()) {
    LatexGenerator generator(file_name, content, context, tree);
    generator.Generate();

    std::ofstream out(GenerateOutputPath(file_name, itr->second, "tex"));
    out << generator.ShowOutput();
  }
}

void Driver::BuildBookFilesMap() {
  for (auto [start_file_number, path] : options_.book_file_and_dir) {
    std::vector<std::string>& book_files =
        book_start_to_remaining_[start_file_number];

    std::string_view file_number = start_file_number;
    while (true) {
      std::optional<std::string> file_name =
          NextPageToActualFileName(std::string(file_number));
      if (!file_name) {
        break;
      }
      book_dir_to_files_[*file_name] = path;
      book_files.push_back(*file_name);

      const Metadata* metadata = repo_.FindMetadataByFilename(*file_name);
      if (metadata == nullptr) {
        break;
      }

      file_number = metadata->GetNextPage();
      if (file_number.empty()) {
        break;
      }
    }
  }
}

void Driver::GenerateBookMainPage() const {
  for (auto [start_file_number, path] : options_.book_file_and_dir) {
    if (auto itr = book_start_to_remaining_.find(start_file_number);
        itr != book_start_to_remaining_.end()) {
      BookGenerator gen;
      std::string tex =
          gen.GenerateMainTex(start_file_number, itr->second, repo_);

      std::ofstream out(GenerateOutputPath("main", path, "tex"));
      out << tex;
    }
  }
}

void Driver::GenerateJSONFiles() const {
  std::ofstream out_path(options_.json_output_dir + "/page_path.json");
  out_path << repo_.DumpPathAsJson();

  std::ofstream out_header(options_.json_output_dir + "/file_headers.json");
  out_header << repo_.DumpFileHeaderAsJson();
}

void Driver::StartClangFormatServer(const std::string& server_location) {
  if (clang_format_server_spanwed_) {
    return;
  }

  clang_format_server_spanwed_ = true;

  int pid = fork();
  if (pid < 0) {
    LOG(0) << "Fork error!";
    return;
  }

  if (pid > 0) {
    clang_format_pid_ = pid;
    return;
  } else {
    std::string port = std::to_string(options_.clang_format_server_port);
    char* server_argv[] = {const_cast<char*>(server_location.c_str()),
                           const_cast<char*>("--port"),
                           const_cast<char*>(port.c_str()), NULL};
    char* server_env[] = {NULL};

    int ret = execve(server_location.c_str(), server_argv, server_env);
    LOG(0) << "Unable to run execve " << server_location
           << " with error code : " << ret;
    clang_format_server_spanwed_ = false;
  }
}

void Driver::UpdateDatabase() const {
  Database database(options_.auth_file_path, repo_);
  for (const auto& [file_name, file_info] : file_contents_) {
    auto& [content, read_pos, rel_path] = file_info;
    database.TryUpdateFileToDatabase(
        std::string(MetadataRepo::NormalizeFileName(file_name)), content);
  }
}

Driver::~Driver() {
  if (clang_format_pid_ != 0) {
    kill(clang_format_pid_, SIGKILL);

    int status;
    waitpid(clang_format_pid_, &status, 0);
  }
}

}  // namespace md2

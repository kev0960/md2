#include "driver.h"

#include <fmt/color.h>
#include <fmt/core.h>

#include <filesystem>
#include <fstream>

#include "generators/html_generator.h"
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
  return StrCat(output_dir, "/", p.stem().c_str(), ".", ext);
}

}  // namespace

void Driver::Run() {
  fmt::print(fmt::fg(fmt::color::red), "Starting driver \n");
  ReadFilesInDirectory();
  BuildFileMetadataRepo();
  DoParse();
}

void Driver::ReadFilesInDirectory() {
  fmt::print(fmt::fg(fmt::color::green), "Reading files... \n");
  for (const auto& dir : options_.input_dirs) {
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
      const fs::path& path = entry.path();

      // Only handle md files.
      if (path.extension() != ".md") {
        continue;
      }

      file_contents_[path.filename()] =
          std::make_pair(ReadFileContent(path), 0);
    }
  }

  for (const auto& file : options_.input_files) {
    const fs::path path(file);
    file_contents_[path.filename()] = std::make_pair(ReadFileContent(path), 0);
  }
}

std::string Driver::ParseFile(std::string_view content) {
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  GeneratorContext context;
  HTMLGenerator generator(content, context);
  generator.Generate(tree);

  return std::move(generator).ReleaseGeneratedTarget();
}

void Driver::BuildFileMetadataRepo() {
  for (auto& [file_name, content_and_pos] : file_contents_) {
    auto& [content, read_pos] = content_and_pos;
    auto metadata_or = MetadataFactory::ParseMetadata(content, read_pos);
    if (metadata_or) {
      repo_.RegisterMetadata(file_name, metadata_or.value());
    }
  }
}

void Driver::DoParse() {
  fmt::print(fmt::fg(fmt::color::green), "Start parsing ... \n");
  ThreadPool pool(options_.num_threads);

  for (auto& [file_name, content_and_pos] : file_contents_) {
    auto& [file_content, pos] = content_and_pos;
    std::string_view content(file_content.c_str() + pos);

    pool.enqueue(
        [this, content](std::string file_name) { DoParse(content, file_name); },
        file_name);
  }
}

void Driver::DoParse(std::string_view content, std::string_view file_name) {
  std::string output_file_name =
      GenerateOutputPath(file_name, options_.output_dir, "html");

  fmt::print("[{}/{}] Generating [{}] to [{}] \n", ++num_parsed_,
             file_contents_.size(), file_name, output_file_name);

  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  GeneratorContext context;
  if (options_.generate_html) {
    HTMLGenerator generator(content, context);
    generator.Generate(tree);

    std::ofstream out(output_file_name);
    out << generator.ShowOutput();
  }
}

}  // namespace md2

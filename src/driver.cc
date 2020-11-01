#include "driver.h"

#include <filesystem>
#include <fstream>

#include "generators/html_generator.h"
#include "parse_tree.h"
#include "parser.h"

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

}  // namespace

void Driver::ReadFilesInDirectory(const std::vector<std::string>& dirs) {
  for (const auto& dir : dirs) {
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
      const fs::path& path = entry.path();

      // Only handle md files.
      if (path.extension() != "md") {
        continue;
      }

      file_contents_[path.filename()] = ReadFileContent(path);
    }
  }
}

std::string Driver::ParseFile(std::string_view content) {
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  HTMLGenerator generator(content);
  generator.Generate(tree);

  return std::move(generator).ReleaseGeneratedTarget();
}

}  // namespace md2

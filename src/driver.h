#ifndef DRIVER_H
#define DRIVER_H

#include <atomic>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "metadata_repo.h"

namespace md2 {

struct DriverOptions {
  std::vector<std::string> input_dirs;
  std::vector<std::string> input_files;

  // Book start file and the tex output directory.
  std::vector<std::pair<std::string, std::string>> book_file_and_dir;

  std::string output_dir;

  std::string image_path;

  bool should_log_db;
  bool generate_html = true;
  bool generate_latex = true;

  size_t num_threads = 8;
};

class Driver {
 public:
  // string file content, pos, rel_path
  using FileInfo = std::tuple<std::string, size_t, std::string>;

  Driver(const DriverOptions& options) : options_(options) {}

  // Run the driver.
  void Run();

 private:
  // Read files in the dirs.
  void ReadFilesInDirectory();

  // Build the file metadata repository.
  void BuildFileMetadataRepo();

  void DoParse();

  // Note that file_name is not a full path (e.g 251.md)
  void DoParse(std::string_view content, std::string_view file_name);

  // Figure out md files that needed to be parsed for book.
  void BuildBookFilesMap();

  // Emit the main tex file for each book.
  void GenerateBookMainPage() const;

  DriverOptions options_;

  // Map between file name to the file content and the current reading position.
  std::unordered_map<std::string, FileInfo> file_contents_;

  MetadataRepo repo_;

  std::atomic<int> num_parsed_ = 0;

  // Map between the file name to the book directory.
  std::unordered_map<std::string, std::string> book_dir_to_files_;

  // Map between the start file and the included tex files in order.
  std::unordered_map<std::string, std::vector<std::string>>
      book_start_to_remaining_;
};

}  // namespace md2

#endif

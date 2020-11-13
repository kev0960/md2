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
  void DoParse(std::string_view content, std::string_view file_name);

  DriverOptions options_;

  // Map between file name to the file content and the current reading position.
  std::unordered_map<std::string, FileInfo> file_contents_;

  MetadataRepo repo_;

  std::atomic<int> num_parsed_ = 0;
};

}  // namespace md2

#endif

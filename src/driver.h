#ifndef DRIVER_H
#define DRIVER_H

#include <string_view>
#include <unordered_map>
#include <vector>

#include "metadata_repo.h"

namespace md2 {

struct DriverOptions {
  std::string output_dir;
  bool should_log_db;
  bool generate_html = true;
  bool generate_latex = true;
};

class Driver {
 public:
  Driver(const DriverOptions& options) : options_(options) {}

  // Read files in the dirs.
  void ReadFilesInDirectory(const std::vector<std::string>& dirs);

  // Build the file metadata repository.
  void BuildFileMetadataRepo();

  std::string ParseFile(std::string_view content);

  void DoParse();

 private:
  DriverOptions options_;

  // Map between file name to the file content and the current reading position.
  std::unordered_map<std::string, std::pair<std::string, int>> file_contents_;

  MetadataRepo repo_;
};

}  // namespace md2

#endif

#ifndef DRIVER_H
#define DRIVER_H

#include <string_view>
#include <unordered_map>
#include <vector>

namespace md2 {

class Driver {
 public:
  // Read files in the dirs.
  void ReadFilesInDirectory(const std::vector<std::string>& dirs);

  // Build the file metadata repository.
  void BuildFileMetadataRepo();

  std::string ParseFile(std::string_view content);

 private:
  // Map between file name to the file content.
  std::unordered_map<std::string, std::string> file_contents_;
};

}  // namespace md2

#endif

#ifndef METADATA_REPO_H
#define METADATA_REPO_H

#include <string>
#include <string_view>
#include <unordered_map>

namespace md2 {

// Repository of every parsed Markdown metadata.
class MetadataRepo {
 public:
  bool RegisterMetadata(
      std::string_view filename,
      const std::unordered_map<std::string, std::string_view>& parsed_data);

 private:
  std::unordered_map<std::string,
                     std::unordered_map<std::string, std::string_view>>
      metadata_;
};

}  // namespace md2
#endif

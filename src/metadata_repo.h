#ifndef METADATA_REPO_H
#define METADATA_REPO_H

#include <string>
#include <string_view>
#include <unordered_map>

#include "metadata.h"

namespace md2 {

// Repository of every parsed Markdown metadata.
class MetadataRepo {
 public:
  bool RegisterMetadata(std::string_view filename, const Metadata& metadata);

  // Find the first match in the metadata.
  const Metadata* FindMetadata(std::string_view ref);
  const Metadata* FindMetadata(std::string_view ref, std::string_view path);

 private:
  std::unordered_map<std::string, Metadata> repo_;
  std::unordered_map<std::string, std::vector<const Metadata*>>
      ref_to_metadata_;
};

}  // namespace md2
#endif

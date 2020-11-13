#ifndef METADATA_REPO_H
#define METADATA_REPO_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "metadata.h"

namespace md2 {

// Repository of every parsed Markdown metadata.
class MetadataRepo {
 public:
  bool RegisterMetadata(std::string_view filename,
                        std::unique_ptr<Metadata> metadata);

  // Find the first match in the metadata.
  const Metadata* FindMetadata(std::string_view ref) const;
  const Metadata* FindMetadata(std::string_view ref,
                               std::string_view path) const;

  const Metadata* FindMetadataByFilename(std::string_view filename) const;

 private:
  std::unordered_map<std::string, std::unique_ptr<Metadata>> repo_;
  std::unordered_map<std::string, std::vector<const Metadata*>>
      ref_to_metadata_;
};

}  // namespace md2
#endif

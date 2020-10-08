#include "metadata_repo.h"

namespace md2 {

bool MetadataRepo::RegisterMetadata(
    std::string_view filename,
    const std::unordered_map<std::string, std::string_view>& parsed_data) {
  metadata_[std::string(filename)] = parsed_data;
}

}  // namespace md2

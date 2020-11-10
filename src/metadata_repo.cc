#include "metadata_repo.h"

#include "logger.h"

namespace md2 {

bool MetadataRepo::RegisterMetadata(std::string_view filename,
                                    std::unique_ptr<Metadata> metadata) {
  // Do not register if the file is already there.
  if (repo_.count(std::string(filename))) {
    return false;
  }

  Metadata* metadata_ptr = metadata.get();
  repo_[std::string(filename)] = std::move(metadata);

  for (const auto& ref : metadata_ptr->GetRefNames()) {
    auto ref_metadata_itr = ref_to_metadata_.find(ref);
    if (ref_metadata_itr == ref_to_metadata_.end()) {
      auto [itr, ok] =
          ref_to_metadata_.insert({ref, std::vector<const Metadata*>()});
      ref_metadata_itr = itr;
    }

    ref_metadata_itr->second.push_back(metadata_ptr);
  }

  return true;
}

const Metadata* MetadataRepo::FindMetadata(std::string_view ref) const {
  auto itr = ref_to_metadata_.find(std::string(ref));
  if (itr == ref_to_metadata_.end()) {
    return nullptr;
  }

  if (itr->second.empty()) {
    return nullptr;
  }

  return itr->second.front();
}

const Metadata* MetadataRepo::FindMetadata(std::string_view ref,
                                           std::string_view path) const {
  auto itr = ref_to_metadata_.find(std::string(ref));
  if (itr == ref_to_metadata_.end()) {
    return nullptr;
  }

  if (itr->second.empty()) {
    return nullptr;
  }

  for (const Metadata* metadata : itr->second) {
    if (metadata->GetPath().find(path) != std::string_view::npos) {
      return metadata;
    }
  }

  return nullptr;
}

}  // namespace md2

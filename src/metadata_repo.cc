#include "metadata_repo.h"

#include <nlohmann/json.hpp>

#include "logger.h"

namespace md2 {
namespace {

using json = nlohmann::json;

std::string NormalizeFileName(std::string_view file_name) {
  size_t name_start = file_name.find_last_of("/");
  if (name_start != std::string_view::npos) {
    file_name = file_name.substr(name_start + 1);
  }

  size_t ext_start = file_name.find_last_of(".");
  if (ext_start != std::string_view::npos) {
    file_name = file_name.substr(0, ext_start);
  }

  // Remove "dump_" prefix.
  if (!file_name.empty() && file_name.substr(0, 5) == "dump_") {
    file_name = file_name.substr(5);
  }

  return std::string(file_name);
}

std::vector<std::string> PathToVec(std::string_view path) {
  std::vector<std::string> paths;
  size_t current = 0;

  while (true) {
    size_t delim = path.find('/', current);
    if (delim == std::string::npos) {
      paths.push_back(std::string(path.substr(current)));
      break;
    }

    paths.push_back(std::string(path.substr(current, delim - current)));
    current = delim + 1;
  }

  return paths;
}

void ConstructPathJson(json& path, const std::vector<std::string>& paths,
                       size_t index, const std::string& file_name) {
  if (paths.size() == index) {
    path["files"].push_back(file_name);
    return;
  }

  // Every path should have "files" field.
  if (path["files"].empty()) {
    path["files"] = json::array();
  }

  ConstructPathJson(path[paths[index]], paths, index + 1, file_name);
}

}  // namespace

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

const Metadata* MetadataRepo::FindMetadataByFilename(
    std::string_view filename) const {
  if (auto itr = repo_.find(std::string(filename)); itr != repo_.end()) {
    return itr->second.get();
  }

  return nullptr;
}

std::string MetadataRepo::DumpFileHeaderAsJson() const {
  json file_headers;
  for (auto& [file_name, metadata] : repo_) {
    json file;
    for (auto& [field_name, field] : metadata->GetAllFields()) {
      file[field_name] = field;
    }
    file_headers[NormalizeFileName(file_name)] = file;
  }

  return file_headers.dump(1);
}

std::string MetadataRepo::DumpPathAsJson() const {
  json path_db;
  for (auto& [file_name, metadata] : repo_) {
    std::string_view path = metadata->GetPath();
    auto paths = PathToVec(path);
    ConstructPathJson(path_db, paths, 0, NormalizeFileName(file_name));
  }

  return path_db.dump(1);
}

}  // namespace md2

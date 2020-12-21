#include "metadata_repo.h"

#include <nlohmann/json.hpp>
#include <unordered_set>

#include "string_util.h"

namespace md2 {
namespace {

using json = nlohmann::json;

std::string_view NormalizeFileName(std::string_view file_name) {
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

  return file_name;
}

std::string ConvertPathnameToFilename(std::string_view path_name) {
  // Instruction name.
  if (!std::all_of(path_name.begin(), path_name.end(), isdigit) ||
      path_name.empty()) {
    return StrCat(path_name, ".md");
  }

  int num = std::stoi(std::string(path_name));
  if (num <= 228) {
    return StrCat("dump_", path_name, ".md");
  }

  return StrCat(path_name, ".md");
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
  std::string lowercase_ref;
  std::transform(ref.begin(), ref.end(), std::back_inserter(lowercase_ref),
                 [](const char c) { return std::tolower(c); });

  auto itr = ref_to_metadata_.find(lowercase_ref);
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
  std::string lowercase_ref;
  std::transform(ref.begin(), ref.end(), std::back_inserter(lowercase_ref),
                 [](const char c) { return std::tolower(c); });

  auto itr = ref_to_metadata_.find(lowercase_ref);
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

const Metadata* MetadataRepo::FindMetadataByPathname(
    std::string_view filename) const {
  if (filename.empty()) {
    return nullptr;
  }

  return FindMetadataByFilename(ConvertPathnameToFilename(filename));
}

std::string MetadataRepo::DumpFileHeaderAsJson() const {
  json file_headers;
  for (auto& [file_name, metadata] : repo_) {
    json file;
    for (auto& [field_name, field] : metadata->GetAllFields()) {
      file[field_name] = field;
    }
    file_headers[std::string(NormalizeFileName(file_name))] = file;
  }

  return file_headers.dump(1);
}

std::string MetadataRepo::DumpPathAsJson() const {
  json path_db;

  std::vector<std::pair<std::string_view, const Metadata*>> file_and_metadata;
  file_and_metadata.reserve(repo_.size());

  for (auto& [file_name, metadata] : repo_) {
    file_and_metadata.emplace_back(file_name, metadata.get());
  }

  // Sort by the published date.
  std::sort(file_and_metadata.begin(), file_and_metadata.end(),
            [](const auto& left, const auto& right) {
              const Metadata* left_meta = left.second;
              const Metadata* right_meta = right.second;
              if (left_meta->GetPublishDate() == right_meta->GetPublishDate()) {
                return left_meta->GetTitle() < right_meta->GetTitle();
              }

              return left_meta->GetPublishDate() < right_meta->GetPublishDate();
            });

  std::unordered_set<std::string_view> handled_files;
  handled_files.reserve(repo_.size());

  for (auto& [file_name, metadata] : file_and_metadata) {
    std::string_view current_file = NormalizeFileName(file_name);
    if (handled_files.count(current_file)) {
      continue;
    }

    // Keep finding the next pages.
    const Metadata* current_meta = metadata;
    while (current_meta != nullptr) {
      std::string_view path = current_meta->GetPath();
      auto paths = PathToVec(path);
      ConstructPathJson(path_db, paths, 0, std::string(current_file));
      handled_files.insert(current_file);

      // Find the next page if exist.
      current_file = current_meta->GetNextPage();
      current_meta = FindMetadataByPathname(current_file);

      // Break from errorneous loop.
      if (handled_files.count(current_file)) {
        break;
      }
    }
  }

  return path_db.dump(1);
}

}  // namespace md2

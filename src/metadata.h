#ifndef METADATA_H
#define METADATA_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace md2 {

class Metadata {
  friend class MetadataFactory;

 public:
  std::string_view GetTitle() const { return title_; }
  std::string_view GetCatTitle() const { return cat_title_; }
  std::string_view GetPath() const { return path_; }
  std::string_view GetPublishDate() const { return publish_date_; }
  std::string_view GetFileName() const { return file_name_; }
  std::string_view GetNextPage() const { return next_page_; }
  std::string_view GetChapter() const { return chapter_; }

  bool IsPublished() const { return is_published_; }
  const std::vector<std::string>& GetRefNames() const { return ref_names_; }
  const std::unordered_map<std::string, std::string> GetAllFields() const {
    return all_fields_;
  }

 private:
  // Title that shows up on the html.
  std::string title_;

  // Title that shows up on the category listing.
  std::string cat_title_;

  // List of references that are listed in this file.
  // NOTE We convert reference name to all lower case.
  std::vector<std::string> ref_names_;

  // Path of this document (in the category, not the actual path in the
  // filesystem).
  std::string path_;

  // Time that this is published.
  std::string publish_date_;

  // True if the document is published. This is default true if not specified.
  bool is_published_ = true;

  // Name of the file (not including the extension).
  std::string file_name_;

  // Get next page.
  std::string next_page_;

  // Get chapter.
  std::string chapter_;

  std::unordered_map<std::string, std::string> all_fields_;
};

class MetadataFactory {
 public:
  // Parse the file content to return the constructed file metadata.
  // end will be set as the end of the parsed file metadata.
  static std::unique_ptr<Metadata> ParseMetadata(std::string_view filename,
                                                 std::string_view content,
                                                 size_t& end);
};

}  // namespace md2

#endif

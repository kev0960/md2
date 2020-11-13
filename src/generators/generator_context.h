#ifndef GENERATORS_GENERATOR_CONTEXT_H
#define GENERATORS_GENERATOR_CONTEXT_H

#include <unordered_map>

#include "metadata_repo.h"
#include "parse_tree_nodes/paragraph.h"

namespace md2 {

// Common object that is shared by generators.
class GeneratorContext {
 public:
  GeneratorContext(const MetadataRepo& repo, const std::string& image_path)
      : repo_(repo), image_path_(image_path) {}

  std::string_view GetClangFormatted(const ParseTreeTextNode* node,
                                     std::string_view md);

  // Find the link to the name (if possible) and the reference name (e.g
  // find$vector --> find). If not found, the nthe file_name (first) would be
  // empty.
  std::pair<std::string_view, std::string_view> FindReference(
      std::string_view name);

  std::string_view FindImage(const std::string& image_url);

 private:
  std::unordered_map<const ParseTreeTextNode*, std::string>
      verbatim_to_formatted_;

  // Map from image link url to the actual path.
  std::unordered_map<std::string, std::string> image_url_to_actual_url_;

  const MetadataRepo& repo_;
  std::string image_path_;
};

}  // namespace md2

#endif

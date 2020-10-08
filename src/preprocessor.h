#include <string_view>
#include <unordered_map>

namespace md2 {

// Reads the Markdown metadata section.
class Preprocessor {
 public:
  Preprocessor();

  // Parses the Markdown Metadata
  std::unordered_map<std::string, std::string_view> GetMarkdownMetadata(
      std::string_view content) const;
};

}  // namespace md2

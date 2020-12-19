#ifndef GENERATORS_OBJDUMP_HIGHLIGHTER_H
#define GENERATORS_OBJDUMP_HIGHLIGHTER_H

#include "syntax_highlighter.h"

namespace md2 {

class ObjdumpHighlighter : public SyntaxHighlighter {
 public:
  ObjdumpHighlighter(std::string_view code, std::string_view language)
      : SyntaxHighlighter(code, language) {
    class_to_style_map_.insert({"in", {{"color", "#6dbeff"}}});
    class_to_style_map_.insert({"rg", {{"color", "#78dce8"}}});
    class_to_style_map_.insert({"u", {{"color", "#b26a2f"}}});
    class_to_style_map_.insert({"lb", {{"color", "#f19798"}}});
    class_to_style_map_.insert({"dr", {{"color", "#ff3133"}}});
  }

  bool ParseCode() override;
  void HandleFunctionHeader(size_t start, size_t end);
  size_t HandleOffset(size_t start, size_t end);

 private:
  void AppendTokenVec(const std::vector<SyntaxToken>& tokens, size_t offset);

  void AppendCurrentToken(SyntaxTokenType current_token, size_t token_start,
                          size_t token_end);
};

}  // namespace md2

#endif

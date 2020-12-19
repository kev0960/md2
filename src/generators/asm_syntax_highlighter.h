#ifndef GENERATORS_ASM_SYNTAX_HIGHLIGHTER_H
#define GENERATORS_ASM_SYNTAX_HIGHLIGHTER_H

#include "syntax_highlighter.h"

namespace md2 {

class AsmSyntaxHighlighter : public SyntaxHighlighter {
 public:
  AsmSyntaxHighlighter(std::string_view code, std::string_view language)
      : SyntaxHighlighter(code, language){
    class_to_style_map_.insert({"in", {{"color", "#6dbeff"}}});
    class_to_style_map_.insert({"rg", {{"color", "#78dce8"}}});
    class_to_style_map_.insert({"u", {{"color", "#b26a2f"}}});
    class_to_style_map_.insert({"lb", {{"color", "#f19798"}}});
    class_to_style_map_.insert({"dr", {{"color", "#ff3133"}}});
  }

  bool ParseCode() override;

 private:
  bool ParseLine(size_t line_start, size_t line_end);
  void AppendCurrentToken(SyntaxTokenType current_token, size_t token_start,
                          size_t token_end, int* identifier_index);
};

}  // namespace md2

#endif

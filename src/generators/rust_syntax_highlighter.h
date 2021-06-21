#ifndef GENERATORS_RUST_SYNTAX_HIGHLIGHTER_H
#define GENERATORS_RUST_SYNTAX_HIGHLIGHTER_H

#include "syntax_highlighter.h"

namespace md2 {

class RustSyntaxHighlighter : public SyntaxHighlighter {
 public:
  RustSyntaxHighlighter(std::string_view code, std::string_view language)
      : SyntaxHighlighter(code, language) {}

  bool ParseCode() override;

 private:
  void AppendCurrentToken(SyntaxTokenType current_token, size_t token_start,
                          size_t token_end);

  size_t HandleStringLiteral(size_t string_literal_start);
  size_t HandleTickOrChar(size_t tick_pos);
};
}  // namespace md2

#endif


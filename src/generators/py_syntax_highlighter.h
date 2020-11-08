#ifndef GENERATORS_PY_SYNTAX_HIGHLIGHTER_H
#define GENERATORS_PY_SYNTAX_HIGHLIGHTER_H

#include "syntax_highlighter.h"

namespace md2 {

class PySyntaxHighlighter : public SyntaxHighlighter {
 public:
  PySyntaxHighlighter(const std::string& code, const std::string& language)
      : SyntaxHighlighter(code, language) {}
  bool ParseCode() override;

 private:
  void AppendCurrentToken(SyntaxTokenType current_token, size_t token_start,
                          size_t token_end);
  size_t HandleStringLiteral(size_t string_literal_start);
};

}  // namespace md2

#endif

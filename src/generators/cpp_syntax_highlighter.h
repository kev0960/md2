#ifndef GENERATORS_CPP_SYNTAX_HIGHLIGHTER_H
#define GENERATORS_CPP_SYNTAX_HIGHLIGHTER_H

// NOTE: These syntax highligheters are copied from md-parser.

#include "syntax_highlighter.h"

namespace md2 {

class CppSyntaxHighlighter : public SyntaxHighlighter {
 public:
  CppSyntaxHighlighter(const std::string& code, const std::string& language)
      : SyntaxHighlighter(code, language) {}
  bool ParseCode() override;

 private:
  void AppendCurrentToken(SyntaxTokenType current_token, size_t token_start,
                          size_t token_end);
  size_t HandleMacro(size_t macro_start);
  size_t HandleStringLiteral(size_t string_literal_start);
};

}  // namespace md2

#endif


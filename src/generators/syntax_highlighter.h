#ifndef GENERATORS_SYNTAX_HIGHLIGHTER_H
#define GENERATORS_SYNTAX_HIGHLIGHTER_H

#include <string>
#include <unordered_map>
#include <vector>

namespace md2 {

enum SyntaxTokenType {
  KEYWORD,
  TYPE_KEYWORD,
  IDENTIFIER,
  NUMERIC_LITERAL,
  STRING_LITERAL,
  BRACKET,
  PARENTHESES,
  BRACE,
  PUNCTUATION,  // ',', ';'
  OPERATOR,
  COMMENT,
  MACRO_HEAD,  // "#include"
  MACRO_BODY,  // "<iostream>"
  WHITESPACE,
  // Python only
  FUNCTION,
  BUILT_IN,        // range, print
  MAGIC_FUNCTION,  // __init__
  // Assembly only
  REGISTER,     // e.g eax
  LABEL,        // e.g func:
  DIRECTIVE,    // e.g .align, .global
  INSTRUCTION,  // e.g mov, add, sub
  // Objdump only
  FUNCTION_SECTION,  // <some_function>
  // Rust only
  LIFETIME,
  MACRO,
  NONE  // Not matched to any token.
};

struct SyntaxToken {
  SyntaxTokenType token_type;
  size_t token_start;
  size_t token_end;  // Not inclusive.

  // If this is true then this token will not be color-merged.
  bool no_merge;

  SyntaxToken(SyntaxTokenType token_type, size_t token_start, size_t token_end,
              bool no_merge = false)
      : token_type(token_type),
        token_start(token_start),
        token_end(token_end),
        no_merge(no_merge) {}

  bool operator==(const SyntaxToken& token) const {
    return token_type == token.token_type && token_start == token.token_start &&
           token_end == token.token_end;
  }

  void Print() const;
};

class SyntaxHighlighter {
 public:
  SyntaxHighlighter(std::string_view code, std::string_view language)
      : code_(code), language_(language) {
    class_to_style_map_.insert({"k", {{"color", "#ff6188"}}});
    class_to_style_map_.insert({"s", {{"color", "#ffd866"}}});
    class_to_style_map_.insert({"m", {{"color", "#ff6188"}}});
    class_to_style_map_.insert({"mb", {{"color", "#ffd866"}}});
    class_to_style_map_.insert({"t", {{"color", "#78dce8"}}});
    class_to_style_map_.insert({"c", {{"color", "#727072"}}});
    class_to_style_map_.insert({"o", {{"color", "#ff6188"}}});
    class_to_style_map_.insert({"n", {{"color", "#ab9df2"}}});
    class_to_style_map_.insert({"f", {{"color", "#a9dc76"}}});
    class_to_style_map_.insert({"l", {{"color", "#78dce8"}}});
    class_to_style_map_.insert({"g", {{"color", "#78dce8"}}});
    class_to_style_map_.insert({"mc", {{"color", "#cece1c"}}});
    class_to_style_map_.insert({"lf", {{"color", "#ff9d9d"}}});
    // Background 2d2a2e
  }

  virtual bool ParseCode() { return false; };

  // Merge syntax tokens with same colors.
  void ColorMerge();

  const std::vector<SyntaxToken>& GetTokenList() const;
  std::string GenerateHighlightedHTML() const;
  void OutputColorCss(std::string filename) const;

  virtual std::string GetReferenceOf(SyntaxTokenType type,
                                     std::string_view snippet) const {
    // To disable unused warnings.
    (void)type;

    return std::string(snippet);
  }

  virtual ~SyntaxHighlighter() = default;

 protected:
  void RemoveNewlineInTokenList();

  std::string_view code_;

  std::vector<SyntaxToken> token_list_;
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      class_to_style_map_;

 private:
  std::string_view language_;
};

}  // namespace md2

#endif

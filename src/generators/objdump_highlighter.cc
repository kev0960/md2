#include "objdump_highlighter.h"

#include <algorithm>

#include "asm_syntax_highlighter.h"
#include "cpp_syntax_highlighter.h"
#include "logger.h"
#include "string_util.h"

namespace md2 {
namespace {

constexpr std::string_view kWhiteSpace = " \t";

inline bool IsHexDigit(char c) {
  if ('0' <= c && c <= '9') {
    return true;
  }

  if ('a' <= c && c <= 'f') {
    return true;
  }

  if ('A' <= c && c <= 'F') {
    return true;
  }

  return false;
}

bool IsThisAssembly(std::string_view code) {
  size_t whitespace_prefix_end = code.find_first_not_of(kWhiteSpace);
  if (whitespace_prefix_end == 0) {
    return false;
  }

  size_t offset_end = code.find(':', whitespace_prefix_end + 1);
  if (offset_end == std::string_view::npos) {
    return false;
  }

  std::string_view offset_str =
      code.substr(whitespace_prefix_end, offset_end - 4);

  // Offset str should be all in hex.
  if (std::any_of(offset_str.begin(), offset_str.end(),
                  [](char c) { return !IsHexDigit(c); })) {
    return false;
  }

  return true;
}

bool IsThisFunctionHeader(std::string_view code) {
  size_t offset_end = code.find(' ');
  if (offset_end != 16) {
    return false;
  }

  if (std::any_of(code.begin(), code.begin() + offset_end,
                  [](char c) { return !IsHexDigit(c); })) {
    return false;
  }

  return true;
}

}  // namespace

void ObjdumpHighlighter::AppendTokenVec(const std::vector<SyntaxToken>& tokens,
                                        size_t offset) {
  token_list_.reserve(token_list_.size() + tokens.size());
  for (const auto& token : tokens) {
    token_list_.push_back(SyntaxToken(token.token_type,
                                      token.token_start + offset,
                                      token.token_end + offset));
  }
}

bool ObjdumpHighlighter::ParseCode() {
  size_t current = 0;

  while (current < code_.size()) {
    size_t line_end = code_.find('\n', current);

    if (line_end == std::string_view::npos) {
      line_end = code_.size();
    }

    // current_line does not include the ending newline.
    std::string_view current_line = code_.substr(current, line_end - current);
    if (IsThisAssembly(current_line)) {
      // Handle the offset.
      size_t inst_start = HandleOffset(current, line_end);
      AsmSyntaxHighlighter asm_highlighter(
          code_.substr(inst_start, line_end - inst_start), "asm");
      asm_highlighter.ParseCode();
      AppendTokenVec(asm_highlighter.GetTokenList(), inst_start);
    } else if (IsThisFunctionHeader(current_line)) {
      HandleFunctionHeader(current, line_end);
    } else {
      CppSyntaxHighlighter cpp_highlighter(current_line, "cpp");
      cpp_highlighter.ParseCode();
      AppendTokenVec(cpp_highlighter.GetTokenList(), current);
    }

    if (line_end == code_.size()) {
      break;
    }

    token_list_.push_back(SyntaxToken(WHITESPACE, line_end, line_end + 1));
    current = line_end + 1;
  }

  return true;
}

void ObjdumpHighlighter::HandleFunctionHeader(size_t start, size_t end) {
  token_list_.push_back(SyntaxToken(NUMERIC_LITERAL, start, start + 16));
  token_list_.push_back(SyntaxToken(WHITESPACE, start + 16, start + 17));
  token_list_.push_back(SyntaxToken(FUNCTION_SECTION, start + 17, end));
}

size_t ObjdumpHighlighter::HandleOffset(size_t start, size_t end) {
  size_t whitespace_prefix_end = code_.find_first_not_of(kWhiteSpace, start);
  token_list_.push_back(SyntaxToken(WHITESPACE, start, whitespace_prefix_end));

  size_t offset_end = code_.find(":", whitespace_prefix_end + 1);
  token_list_.push_back(
      SyntaxToken(NUMERIC_LITERAL, whitespace_prefix_end, offset_end + 1));

  // Now fetch the hex instruction part.
  size_t hex_start = code_.find_first_not_of(kWhiteSpace, offset_end + 1);
  token_list_.push_back(SyntaxToken(WHITESPACE, offset_end + 1, hex_start));

  while (true) {
    size_t hex_end = code_.find_first_of(kWhiteSpace, hex_start + 1);
    std::string_view maybe_hex = code_.substr(hex_start, hex_end - hex_start);
    if (!std::all_of(maybe_hex.begin(), maybe_hex.end(), IsHexDigit)) {
      return hex_start;
    }

    token_list_.push_back(SyntaxToken(NUMERIC_LITERAL, hex_start, hex_end));

    hex_start = code_.find_first_not_of(kWhiteSpace, hex_end + 1);

    if (hex_start == std::string_view::npos) {
      hex_start = code_.size();
    }

    token_list_.push_back(SyntaxToken(WHITESPACE, hex_end, hex_start));

    if (hex_start - hex_end != 1 || hex_start >= end) {
      // Then this is the start of the real opcode.
      return hex_start;
    }
  }
  return hex_start;
}

}  // namespace md2

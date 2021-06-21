#include "rust_syntax_highlighter.h"

#include <unordered_set>

#include "container_util.h"
#include "logger.h"
#include "string_util.h"

namespace md2 {
namespace {

std::unordered_set<std::string_view> kRustKeywords = {
    "as",     "break",  "const", "continue", "crate",  "else",   "enum",
    "extern", "false",  "fn",    "for",      "if",     "impl",   "in",
    "let",    "loop",   "match", "mod",      "move",   "mut",    "pub",
    "ref",    "return", "self",  "Self",     "static", "struct", "super",
    "trait",  "true",   "type",  "unsafe",   "use",    "where",  "while",
    "async",  "await",  "dyn"};

std::unordered_set<std::string_view> kRustTypeKeywords = {
    "i8",   "u8",    "i16",   "u16", "i32", "u32", "i64", "u64",   "i128",
    "u128", "isize", "usize", "f32", "f64", "str", "Vec", "String"};

// Check whether the character is allowed in the identifier.
bool IsIdenfierAllowedChar(char c) {
  if ('0' <= c && c <= '9') {
    return true;
  }
  if ('a' <= c && c <= 'z') {
    return true;
  }
  if ('A' <= c && c <= 'Z') {
    return true;
  }
  if (c == '_') {
    return true;
  }
  return false;
}

bool IsWhiteSpace(char c) {
  if (c == '\t' || c == ' ' || c == '\n') {
    return true;
  }
  return false;
}

bool IsNumber(char c) { return '0' <= c && c <= '9'; }

bool IsNumericLiteral(std::string_view s) {
  if (s.length() == 0) {
    return false;
  }
  if (IsNumber(s[0])) {
    return true;
  }
  if (s[0] == '.' && s.length() > 1) {
    if (IsNumber(s[1])) {
      return true;
    }
    return false;
  }
  return false;
}

bool IsParentheses(char c) {
  if (c == '(' || c == ')') {
    return true;
  }
  return false;
}

bool IsStringLiteralStart(char c) {
  if (c == '"') {
    return true;
  }
  return false;
}

bool IsTick(char c) {
  if (c == '\'') {
    return true;
  }
  return false;
}

bool IsOperator(char c) {
  switch (c) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
    case '&':
    case '!':
    case '?':
    case ':':
    case '|':
    case '=':
    case '<':
    case '>':
    case '~':
    case '.':
    case '#':
      return true;
  }
  return false;
}

bool IsPunctuation(char c) {
  if (c == ',' || c == ';') {
    return true;
  }
  return false;
}

bool IsBracket(char c) {
  if (c == '[' || c == ']') {
    return true;
  }
  return false;
}

bool IsBrace(char c) {
  if (c == '{' || c == '}') {
    return true;
  }
  return false;
}

}  // namespace

bool RustSyntaxHighlighter::ParseCode() {
  SyntaxTokenType current_token = NONE;
  size_t token_start = 0;

  for (size_t i = 0; i < code_.length(); i++) {
    char c = code_[i];

    if (c == '/' && i < code_.length() - 1) {
      if (code_[i + 1] == '/') {
        // Then this is the start of the comment.
        AppendCurrentToken(current_token, token_start, i);
        current_token = NONE;

        size_t comment_end = code_.find("\n", i + 2);
        if (comment_end == std::string::npos) {
          comment_end = code_.length();
        }
        token_list_.emplace_back(COMMENT, i, comment_end);
        i = comment_end - 1;
        continue;
      }
    }

    if (IsOperator(c)) {
      // '.' can come in the middle of the floating point number.
      if (c == '.' && current_token == IDENTIFIER &&
          IsNumericLiteral(code_.substr(token_start, i - token_start))) {
        continue;
      }

      if (current_token != OPERATOR) {
        AppendCurrentToken(current_token, token_start, i);
        current_token = OPERATOR;
        token_start = i;
      }
    }

    // During tokenizing step, we do not distinguish between identifier versus
    // keyword. After the identifier token has determined, we check whether it
    // matches to one of our keyword set. If it does, then we mark it as a
    // keyword.
    else if (IsIdenfierAllowedChar(c)) {
      if (current_token != IDENTIFIER) {
        AppendCurrentToken(current_token, token_start, i);
        current_token = IDENTIFIER;
        token_start = i;
      }
    } else if (IsWhiteSpace(c)) {
      if (current_token != WHITESPACE) {
        AppendCurrentToken(current_token, token_start, i);
        current_token = WHITESPACE;
        token_start = i;
      }
    } else if (IsParentheses(c)) {
      if (current_token != PARENTHESES) {
        AppendCurrentToken(current_token, token_start, i);
        current_token = PARENTHESES;
        token_start = i;
      }
    } else if (IsStringLiteralStart(c)) {
      AppendCurrentToken(current_token, token_start, i);
      token_start = HandleStringLiteral(i);
      i = token_start - 1;
      current_token = NONE;
    } else if (IsBracket(c)) {
      if (current_token != BRACKET) {
        AppendCurrentToken(current_token, token_start, i);
        current_token = BRACKET;
        token_start = i;
      }
    } else if (IsBrace(c)) {
      if (current_token != BRACE) {
        AppendCurrentToken(current_token, token_start, i);
        current_token = BRACE;
        token_start = i;
      }
    } else if (IsPunctuation(c)) {
      if (current_token != PUNCTUATION) {
        AppendCurrentToken(current_token, token_start, i);
        current_token = PUNCTUATION;
        token_start = i;
      }
    } else if (IsTick(c)) {
      AppendCurrentToken(current_token, token_start, i);
      token_start = HandleTickOrChar(i);
      i = token_start - 1;
      current_token = NONE;
    }
  }

  AppendCurrentToken(current_token, token_start, code_.length());

  // If some identifier has ! after it, then it is a macro.
  // If ( is followed then it is a function.
  for (size_t i = 0; i < token_list_.size(); i++) {
    if (token_list_[i].token_type == IDENTIFIER && i < token_list_.size() - 1) {
      if (i + 1 < token_list_.size() &&
          token_list_[i + 1].token_type == OPERATOR) {
        auto token = token_list_[i + 1];
        if (code_.substr(token.token_start,
                         token.token_end - token.token_start) == "!") {
          token_list_[i].token_type = MACRO;
          token_list_[i + 1].token_type = MACRO;
          continue;
        }
      }

      // Ignore Whitespaces.
      size_t next_parenth = i + 1;
      while (next_parenth < token_list_.size() &&
             token_list_[next_parenth].token_type == WHITESPACE) {
        next_parenth++;
      }
      if (next_parenth < token_list_.size() &&
          token_list_[next_parenth].token_type == PARENTHESES &&
          code_[token_list_[next_parenth].token_start] == '(') {
        token_list_[i].token_type = FUNCTION;
      }
    }
  }
  return true;
}

size_t RustSyntaxHighlighter::HandleTickOrChar(size_t tick_pos) {
  // Single char.
  if (tick_pos + 2 < code_.size() && code_[tick_pos + 2] == '\'') {
    AppendCurrentToken(STRING_LITERAL, tick_pos, tick_pos + 3);
    return tick_pos + 3;
  } else {
    size_t tick_pos_start = tick_pos;
    tick_pos += 1;

    // Read until non alphabet.
    while (tick_pos < code_.size() && IsIdenfierAllowedChar(code_[tick_pos])) {
      tick_pos++;
    }

    AppendCurrentToken(LIFETIME, tick_pos_start, tick_pos);
    return tick_pos;
  }

  return code_.size();
}

size_t RustSyntaxHighlighter::HandleStringLiteral(size_t string_literal_start) {
  // Check raw string literal.
  size_t raw_string_start = string_literal_start;
  while (raw_string_start > 0 && code_[raw_string_start - 1] == '#') {
    raw_string_start--;
  }

  std::string raw_string_marker;
  if (raw_string_start > 0 && code_[raw_string_start - 1] == 'r') {
    raw_string_marker =
        code_.substr(raw_string_start, string_literal_start - raw_string_start);
    string_literal_start = raw_string_start - 1;
  }

  size_t string_literal_end =
      string_literal_start +
      (raw_string_marker.length() == 0 ? 0 : raw_string_marker.length() + 1);

  while (string_literal_end < code_.length()) {
    size_t end = code_.find('"', string_literal_end + 1);
    if (end == std::string::npos) {
      break;
    }

    if (end == code_.length() - 1) {
      break;
    }

    // Escaped ".
    if (code_[end - 1] == '\\') {
      string_literal_end = end;
      continue;
    }

    if (code_.substr(end + 1, raw_string_marker.size()) == raw_string_marker) {
      string_literal_end = end + 1 + raw_string_marker.size();
      if (code_[string_literal_start] == 'r') {
        token_list_.pop_back();
      }

      if (!raw_string_marker.empty()) {
        token_list_.pop_back();
      }

      AppendCurrentToken(STRING_LITERAL, string_literal_start,
                         string_literal_end);

      return string_literal_end;
    }

    string_literal_end = end;
  }

  AppendCurrentToken(STRING_LITERAL, string_literal_start, code_.length());
  return code_.length();
}

void RustSyntaxHighlighter::AppendCurrentToken(SyntaxTokenType current_token,
                                               size_t token_start,
                                               size_t token_end) {
  if (current_token == IDENTIFIER) {
    // Check whether it matches one of our keyword set.
    std::string_view token = code_.substr(token_start, token_end - token_start);
    if (SetContains(kRustKeywords, token)) {
      current_token = KEYWORD;
    } else if (SetContains(kRustTypeKeywords, token)) {
      current_token = TYPE_KEYWORD;
    } else if (IsNumericLiteral(token)) {
      current_token = NUMERIC_LITERAL;
    }

    token_list_.emplace_back(current_token, token_start, token_end);
  } else if (current_token != NONE) {
    token_list_.emplace_back(current_token, token_start, token_end);
  }
}

}  // namespace md2

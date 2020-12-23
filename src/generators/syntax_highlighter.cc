#include "syntax_highlighter.h"

#include <algorithm>
#include <fstream>

#include "logger.h"
#include "string_util.h"

namespace md2 {
namespace {

std::string TokenTypeToClassName(const SyntaxTokenType token_type) {
  switch (token_type) {
    case KEYWORD:
      return "k";
    case TYPE_KEYWORD:
      return "t";
    case IDENTIFIER:
      return "i";
    case NUMERIC_LITERAL:
      return "n";
    case STRING_LITERAL:
      return "s";
    case BRACKET:
      return "b";
    case PARENTHESES:
      return "p";
    case PUNCTUATION:
      return "u";
    case OPERATOR:
      return "o";
    case COMMENT:
      return "c";
    case MACRO_HEAD:
      return "m";
    case MACRO_BODY:
      return "mb";
    case WHITESPACE:
      return "w";
    case BRACE:
      return "r";
    case FUNCTION:
      return "f";
    case BUILT_IN:
      return "l";
    case MAGIC_FUNCTION:
      return "g";
    case REGISTER:
      return "rg";
    case LABEL:
      return "lb";
    case DIRECTIVE:
      return "dr";
    case INSTRUCTION:
      return "in";
    case FUNCTION_SECTION:
      return "fs";
    case NONE:
      return "";
  }
  return "";
}

std::string_view TokenTypeToString(SyntaxTokenType type) {
  switch (type) {
    case KEYWORD:
      return "KEYWORD";
    case TYPE_KEYWORD:
      return "TYPE_KEYWORD";
    case IDENTIFIER:
      return "IDENTIFIER";
    case NUMERIC_LITERAL:
      return "NUMERIC_LITERAL";
    case STRING_LITERAL:
      return "STRING_LITERAL";
    case BRACKET:
      return "BRACKET";
    case PARENTHESES:
      return "PARENTHESES";
    case BRACE:
      return "BRACE";
    case PUNCTUATION:
      return "PUNCTUATION";
    case OPERATOR:
      return "OPERATOR";
    case COMMENT:
      return "COMMENT";
    case MACRO_HEAD:
      return "MACRO_HEAD";
    case MACRO_BODY:
      return "MACRO_BODY";
    case WHITESPACE:
      return "WHITESPACE";
    case FUNCTION:
      return "FUNCTION";
    case BUILT_IN:
      return "BUILT_IN";
    case MAGIC_FUNCTION:
      return "MAGIC_FUNCTION";
    case REGISTER:
      return "REGISTER";
    case LABEL:
      return "LABEL";
    case DIRECTIVE:
      return "DIRECTIVE";
    case INSTRUCTION:
      return "INSTRUCTION";
    case FUNCTION_SECTION:
      return "FUNCTION_SECTION";
    case NONE:
      return "NONE";
  }
  return "";
}

void EscapeHTML(std::string* s) {
  for (size_t i = 0; i < s->length(); i++) {
    if (s->at(i) == '<') {
      s->replace(i, 1, "&lt;");
      i += 3;
    } else if (s->at(i) == '>') {
      s->replace(i, 1, "&gt;");
      i += 3;
    } else if (s->at(i) == '&') {
      s->replace(i, 1, "&amp;");
      i += 3;
    }
  }
}

template <typename K, typename V>
bool IsUnorderedMapIdentical(const std::unordered_map<K, V>& m1,
                             const std::unordered_map<K, V>& m2) {
  if (m1.size() != m2.size()) {
    return false;
  }
  for (const auto& kv : m1) {
    if (m2.find(kv.first) == m2.end()) {
      return false;
    }
    if (kv.second != m2.at(kv.first)) {
      return false;
    }
  }
  return true;
}

}  // namespace

void SyntaxToken::Print() const {
  LOG(0) << TokenTypeToString(token_type) << " [" << token_start << " , "
         << token_end << "]";
}

std::string SyntaxHighlighter::GenerateHighlightedHTML() const {
  std::string html = StrCat("<pre class='chroma lang-", language_, "'>");

  for (const auto& token : token_list_) {
    std::string class_name = TokenTypeToClassName(token.token_type);
    std::string token_str(
        code_.substr(token.token_start, token.token_end - token.token_start));

    EscapeHTML(&token_str);
    html += StrCat("<span class='", class_name, "'>",
                   GetReferenceOf(token.token_type, token_str), "</span>");
  }
  html += "</pre>";

  return html;
}

void SyntaxHighlighter::OutputColorCss(std::string filename) const {
  std::ofstream out(filename);
  for (const auto& [class_name, css] : class_to_style_map_) {
    out << ".chroma ." << class_name << " {\n";
    for (const auto& [style, value] : css) {
      out << "  " << style << ":" << value << ";\n";
    }
    out << "}\n";
  }
}

void SyntaxHighlighter::RemoveNewlineInTokenList() {
  // Ignore first newlines.
  size_t i = 0;
  while (i < token_list_.size() && token_list_[i].token_type == WHITESPACE) {
    std::string_view tok =
        code_.substr(token_list_[i].token_start,
                     token_list_[i].token_end - token_list_[i].token_start);
    for (char c : tok) {
      if (c != '\n') {
        return;
      }
    }
    token_list_.erase(token_list_.begin() + i);
    i++;
  }
}

// Merge the syntax tokens that belongs to same style. This will help to reduce
// the number of DOM elements.
void SyntaxHighlighter::ColorMerge() {
  if (token_list_.empty()) {
    return;
  }

  RemoveNewlineInTokenList();

  // First build a set that tells what Token Types belongs to same style.
  int current_id = 0;
  std::unordered_map<SyntaxTokenType, int> token_type_to_cluster_id;
  for (int token_type = KEYWORD; token_type != NONE; token_type++) {
    const auto tt = static_cast<SyntaxTokenType>(token_type);
    bool added = false;
    for (const auto& kv : token_type_to_cluster_id) {
      if (IsUnorderedMapIdentical(
              class_to_style_map_[TokenTypeToClassName(tt)],
              class_to_style_map_[TokenTypeToClassName(kv.first)])) {
        token_type_to_cluster_id.insert({tt, kv.second});
        added = true;
        break;
      }
    }
    if (!added) {
      token_type_to_cluster_id.insert({tt, current_id++});
    }
  }

  // Now iterate through the token list and merge the tokens with same style.
  size_t i = 0;
  while (i < token_list_.size() - 1) {
    if (token_list_[i].no_merge) {
      i++;
      continue;
    }

    const auto current_token = token_list_[i].token_type;
    size_t pivot = i;
    while (i < token_list_.size() - 1) {
      if (token_list_[i + 1].no_merge) {
        break;
      }

      const auto next_token = token_list_[i + 1].token_type;
      if (token_type_to_cluster_id[current_token] ==
          token_type_to_cluster_id[next_token]) {
        token_list_[pivot].token_end = token_list_[i + 1].token_end;
        token_list_[i + 1].token_start = token_list_[i + 1].token_end;
        i++;
      } else {
        break;
      }
    }
    i++;
  }

  // Now remove all the empty string tokens.
  token_list_.erase(std::remove_if(token_list_.begin(), token_list_.end(),
                                   [](const SyntaxToken& token) {
                                     return token.token_start ==
                                            token.token_end;
                                   }),
                    token_list_.end());
}

const std::vector<SyntaxToken>& SyntaxHighlighter::GetTokenList() const {
  return token_list_;
}

}  // namespace md2

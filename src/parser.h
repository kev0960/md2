#ifndef PARSER_H
#define PARSER_H

#include "parse_tree.h"
#include <string>

namespace md2 {

// Parses Markdown and emits the parse tree.
//
// The parsed syntax tree will be converted to the target language (e.g html or
// LaTeX) by the generator.
class Parser {
  public:
    ParseTree GenerateParseTree(std::string_view content);
};

}  // namespace md2

#endif

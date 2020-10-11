#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "parse_tree.h"

namespace md2 {

// Parses Markdown and emits the parse tree.
//
// The parsed syntax tree will be converted to the target language (e.g html or
// LaTeX) by the generator.
class Parser {
 public:
  ParseTree GenerateParseTree(std::string_view content);

 private:
  // Builds the tree from the given root node. Note that this parses until it
  // sees the end_parsing_token. If end_parsing_token is empty, then it tries to
  // parse until the end of the content.
  // Returns the location when the parsing is done.
  int GenericParser(std::string_view content, int start,
                     std::string_view end_parsing_token, ParseTreeNode* root
                     );

  // Try to parse the markdown that starts with '['. The parsing can fail if it
  // does not construct the proper link. In such case, this will return nullptr
  // and end will not be modified.
  // Otherwise, return the constructed tree node and set the end of the parsed
  // link.
  ParseTreeNode* MaybeParseLink(std::string_view content, int start, int& end);
};

}  // namespace md2

#endif

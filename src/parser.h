#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "parse_tree.h"
#include "parse_tree_nodes/image.h"

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
  // Returns the location RIGHT AFTER the end parsing token.
  int GenericParser(std::string_view content, int start,
                    std::string_view end_parsing_token, ParseTreeNode* root);

  // Try to parse the markdown that starts with '['. The parsing can fail if it
  // does not construct the proper link. In such case, this will return nullptr
  // and end will not be modified.
  // Otherwise, return the constructed tree node and set the end of the parsed
  // link.
  template <typename LinkNodeType>
  std::unique_ptr<ParseTreeNode> MaybeParseLink(std::string_view content,
                                                int start, int& end);

  // Parse the image description of the Image node.
  void ParseImageDescriptionMetadata(std::string_view content,
                                     ParseTreeImageNode* image);

  // Try to parse the header that starts with '#'. Note that header must be
  // first in the line (so the preceding character must be '\n'.
  std::unique_ptr<ParseTreeNode> MaybeParseHeader(std::string_view content,
                                                  ParseTreeNode* parent,
                                                  int start, int& end);

  // Try to parse a box (starts with ```).
  std::unique_ptr<ParseTreeNode> MaybeParseBox(std::string_view content,
                                               ParseTreeNode* parent, int start,
                                               int& end);

  // Try to parse the table.
  std::unique_ptr<ParseTreeNode> MaybeParseTable(std::string_view content,
                                                 ParseTreeNode* parent,
                                                 int start, int& end);
  // Try to parse the list.
  std::unique_ptr<ParseTreeNode> MaybeParseList(std::string_view content,
                                                ParseTreeNode* parent,
                                                int start, int& end);

  // Construct List node from the consecutive list items.
  void PostProcessList(ParseTreeNode* root);
};

}  // namespace md2

#endif

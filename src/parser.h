#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "parse_tree.h"
#include "parse_tree_nodes/image.h"

namespace md2 {

// Map between the ref name and the actual node.
using RefContainer = std::unordered_map<std::string, ParseTreeNode*>;

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
  // If use_text is true, then it will use TEXT instead of PARAGRAPH.
  // If must_inline is set, then seeing "\n" in the middle of processing will
  // incur the error.
  size_t GenericParser(std::string_view content, size_t start,
                       std::string_view end_parsing_token, ParseTreeNode* root,
                       RefContainer* refs, bool use_text = false,
                       bool must_inline = false, bool no_link = false);

  // Try to parse the markdown that starts with '['. The parsing can fail if it
  // does not construct the proper link. In such case, this will return nullptr
  // and end will not be modified.
  // Otherwise, return the constructed tree node and set the end of the parsed
  // link.
  template <typename LinkNodeType>
  std::unique_ptr<ParseTreeNode> MaybeParseLink(std::string_view content,
                                                RefContainer* refs,
                                                size_t start, size_t& end);

  // Parse the image description of the Image node.
  void ParseImageDescriptionMetadata(std::string_view content,
                                     ParseTreeImageNode* image);

  // Try to parse the header that starts with '#'. Note that header must be
  // first in the line (so the preceding character must be '\n'.
  std::unique_ptr<ParseTreeNode> MaybeParseHeader(std::string_view content,
                                                  ParseTreeNode* parent,
                                                  RefContainer* refs,
                                                  size_t start, size_t& end);

  // Try to parse a box (starts with ```).
  std::unique_ptr<ParseTreeNode> MaybeParseBox(std::string_view content,
                                               ParseTreeNode* parent,
                                               RefContainer* refs, size_t start,
                                               size_t& end);

  // Try to parse the table.
  std::unique_ptr<ParseTreeNode> MaybeParseTable(std::string_view content,
                                                 ParseTreeNode* parent,
                                                 RefContainer* refs,
                                                 size_t start, size_t& end);

  // Try to parse the list.
  std::unique_ptr<ParseTreeNode> MaybeParseList(std::string_view content,
                                                ParseTreeNode* parent,
                                                RefContainer* refs,
                                                size_t start, size_t& end);

  // Try to parse the command.
  std::unique_ptr<ParseTreeNode> MaybeParseCommand(std::string_view content,
                                                   ParseTreeNode* parent,
                                                   RefContainer* refs,
                                                   size_t start, size_t& end);

  // Try to parse the quote.
  std::unique_ptr<ParseTreeNode> MaybeParseQuote(std::string_view content,
                                                 ParseTreeNode* parent,
                                                 RefContainer* refs,
                                                 size_t start, size_t& end);

  // Construct List node from the consecutive list items.
  void PostProcessList(ParseTreeNode* root);
};

}  // namespace md2

#endif

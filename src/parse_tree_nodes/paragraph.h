#ifndef PARSE_TREE_NODES_PARAGRAPH_H
#define PARSE_TREE_NODES_PARAGRAPH_H

#include "../parse_tree.h"

namespace md2 {

// This node represents the single paragraph.
class ParseTreeParagraphNode : public ParseTreeNode {
 public:
  ParseTreeParagraphNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::PARAGRAPH; }
};

// This node represents the simple text (not part of the paragraph).
class ParseTreeTextNode : public ParseTreeNode {
 public:
  ParseTreeTextNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::TEXT; }
};

}  // namespace md2

#endif

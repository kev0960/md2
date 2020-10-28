#ifndef PARSE_TREE_NODES_TEXT_DECORATION_H
#define PARSE_TREE_NODES_TEXT_DECORATION_H

#include "../parse_tree.h"

namespace md2 {

class ParseTreeBoldNode : public ParseTreeNode {
 public:
  ParseTreeBoldNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::BOLD; }
};

class ParseTreeItalicNode : public ParseTreeNode {
 public:
  ParseTreeItalicNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::ITALIC; }
};

}  // namespace md2

#endif

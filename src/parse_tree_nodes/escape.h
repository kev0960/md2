#ifndef PARSE_TREE_ESCAPE_H
#define PARSE_TREE_ESCAPE_H

#include "node.h"

namespace md2 {

// Node for escaped character. (e.g \*).
class ParseTreeEscapeNode : public ParseTreeNode {
 public:
  ParseTreeEscapeNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start, /*is_leaf_node=*/true) {}

  NodeType GetNodeType() const override { return ParseTreeNode::ESCAPE; }
};

}  // namespace md2

#endif

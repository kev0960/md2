#ifndef PARSE_TREE_LINK_H
#define PARSE_TREE_LINK_H

#include "node.h"

namespace md2 {

// Node for escaped character. (e.g \*).
class ParseTreeLinkNode : public ParseTreeNode {
 public:
  ParseTreeLinkNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::LINK; }
};

}  // namespace md2

#endif

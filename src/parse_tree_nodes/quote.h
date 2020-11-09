#ifndef PARSE_TREE_QUOTE_H
#define PARSE_TREE_QUOTE_H

#include "node.h"

namespace md2 {

class ParseTreeQuoteNode : public ParseTreeNode {
 public:
  ParseTreeQuoteNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::QUOTE; }
};

}  // namespace md2

#endif


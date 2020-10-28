#ifndef PARSE_TREE_NODES_BOX_H
#define PARSE_TREE_NODES_BOX_H

#include "../parse_tree.h"

namespace md2 {

class ParseTreeBoxNode : public ParseTreeNode {
 public:
  ParseTreeBoxNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::BOX; }
};

}  // namespace md2

#endif

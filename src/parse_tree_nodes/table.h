#ifndef PARSE_TREE_NODE_TABLE_H
#define PARSE_TREE_NODE_TABLE_H

#include <vector>

#include "node.h"

namespace md2 {

class ParseTreeTableNode : public ParseTreeNode {
 public:
  ParseTreeTableNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::TABLE; }

  void Generate(Generator* generator) const override;

  // The number of current children will be set as the row size.
  // Will be only set once.
  void SetRowSizeIfNotSpecified();

 private:
  int row_size_ = 0;
};

}  // namespace md2

#endif

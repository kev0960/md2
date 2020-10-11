#ifndef PARSE_TREE_IMAGE_H
#define PARSE_TREE_IMAGE_H

#include "node.h"

namespace md2 {

class ParseTreeImageNode : public ParseTreeNode {
 public:
  ParseTreeImageNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::IMAGE; }
  void Generate(Generator* generator) const override;
};

}  // namespace md2

#endif

#ifndef PARSE_TREE_NODES_VERBATIM_H
#define PARSE_TREE_NODES_VERBATIM_H

#include "../parse_tree.h"

namespace md2 {

class ParseTreeVerbatimNode : public ParseTreeNode {
 public:
  ParseTreeVerbatimNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::VERBATIM; }
  void Generate(Generator* generator) const override;
};

}  // namespace md2

#endif

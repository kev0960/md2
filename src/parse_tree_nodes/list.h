#ifndef PARSE_TREE_NODES_LIST_H
#define PARSE_TREE_NODES_LIST_H

#include "../parse_tree.h"

namespace md2 {

class ParseTreeListNode : public ParseTreeNode {
 public:
  ParseTreeListNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::LIST; }
  void Generate(Generator* generator) const override;
};

class ParseTreeListItemNode : public ParseTreeNode {
 public:
  ParseTreeListItemNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::LIST_ITEM; }
  void Generate(Generator* generator) const override;

  void SetListDepth(int list_depth) { list_depth_ = list_depth; }
  int GetListDepth() const { return list_depth_; }

 private:
  int list_depth_;  // Number of spaces before '*'.
};

}  // namespace md2

#endif

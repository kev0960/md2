#ifndef PARSE_TREE_NODES_LIST_H
#define PARSE_TREE_NODES_LIST_H

#include "../parse_tree.h"

namespace md2 {

class ParseTreeListNode : public ParseTreeNode {
 public:
  ParseTreeListNode(ParseTreeNode* parent, int start, bool is_ordered)
      : ParseTreeNode(parent, start), is_ordered_(is_ordered) {}

  NodeType GetNodeType() const override {
    if (is_ordered_) {
      return ParseTreeNode::ORDERED_LIST;
    }
    return ParseTreeNode::LIST;
  }

  void Generate(Generator* generator) const override;

 private:
  bool is_ordered_ = false;
};

class ParseTreeListItemNode : public ParseTreeNode {
 public:
  ParseTreeListItemNode(ParseTreeNode* parent, int start, bool is_ordered)
      : ParseTreeNode(parent, start), is_ordered_(is_ordered) {}

  NodeType GetNodeType() const override {
    if (is_ordered_) {
      return ParseTreeNode::ORDERED_LIST_ITEM;
    }

    return ParseTreeNode::LIST_ITEM;
  }
  void Generate(Generator* generator) const override;

  void SetListDepth(int list_depth) { list_depth_ = list_depth; }
  int GetListDepth() const { return list_depth_; }

 private:
  int list_depth_;  // Number of spaces before '*'.
  bool is_ordered_ = false;
};

}  // namespace md2

#endif

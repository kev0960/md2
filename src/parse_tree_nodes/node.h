#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "assert.h"

namespace md2 {

class ParseTreeNode {
 public:
  enum NodeType {
    NODE,
    PARAGRAPH,
    TEXT,
    VERBATIM,
    BOLD,
    ITALIC,
    ESCAPE,
    LINK,
    IMAGE,
    HEADER,
    BOX,
    TABLE,
    LIST,
    LIST_ITEM,
    ORDERED_LIST,
    ORDERED_LIST_ITEM,
    COMMAND,
    STRIKE_THROUGH,
    MATH,
    QUOTE
  };

  ParseTreeNode(ParseTreeNode* parent, int start, bool is_leaf_node = false)
      : parent_(parent), start_(start), is_leaf_node_(is_leaf_node) {}

  virtual NodeType GetNodeType() const { return NODE; }
  void SetStart(int start) { start_ = start; }
  void SetEnd(int end) { end_ = end; }

  // Override in case adding a children is not valid.
  void AddChildren(std::unique_ptr<ParseTreeNode> child) {
    if (is_leaf_node_) {
      ASSERT(false, "This node is the leaf node.");
      return;
    }

    children_.push_back(std::move(child));
  }

  void AddChildrenFront(std::unique_ptr<ParseTreeNode> child) {
    if (is_leaf_node_) {
      ASSERT(false, "This node is the leaf node.");
      return;
    }

    children_.insert(children_.begin(), std::move(child));
  }

  // Add a child node right before node_to_find.
  void AddChildBefore(ParseTreeNode* node_to_find,
                      std::unique_ptr<ParseTreeNode> child);

  void SetParent(ParseTreeNode* parent) { parent_ = parent; }
  constexpr ParseTreeNode* GetParent() const { return parent_; }
  ParseTreeNode* GetLastChildren() {
    if (children_.empty()) {
      return nullptr;
    }

    return children_.back().get();
  }

  constexpr const std::vector<std::unique_ptr<ParseTreeNode>>& GetChildren()
      const {
    return children_;
  }

  std::vector<std::unique_ptr<ParseTreeNode>>& GetChildren() {
    return children_;
  }

  std::unique_ptr<ParseTreeNode> PopChildrenAt(int index);

  constexpr int Size() const { return end_ - start_; }
  constexpr int Start() const { return start_; }
  constexpr int End() const { return end_; }

  // Given index, return the TreeNode that contains the pos in its span. If
  // there is no such, then this returns a nullptr.
  ParseTreeNode* GetNext(int pos) const;

  // Same as GetNext but returns the index of the child node instead. If there
  // is no such node, then this returns the children_.size().
  int GetNextChildIndex(int pos) const;

  virtual ~ParseTreeNode() = default;

  void Print(int depth = 0) const;

 protected:
  std::vector<std::unique_ptr<ParseTreeNode>> children_;

  ParseTreeNode* parent_;

  // Spans [start_, end) in the text. Note that end_ is not included.
  int start_;
  int end_;

  // If this is true, then the node cannot have any child node.
  bool is_leaf_node_;
};

}  // namespace md2

#endif

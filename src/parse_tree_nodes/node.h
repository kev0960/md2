#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "../generator.h"

namespace md2 {

class ParseTreeNode {
 public:
  enum NodeType { NODE, PARAGRAPH, BOLD, ITALIC, ESCAPE, LINK};

  ParseTreeNode(ParseTreeNode* parent, int start, bool is_leaf_node = false)
      : parent_(parent), start_(start), is_leaf_node_(is_leaf_node) {}

  virtual NodeType GetNodeType() const { return NODE; }
  void SetEnd(int end) { end_ = end; }

  // Override in case adding a children is not valid.
  void AddChildren(std::unique_ptr<ParseTreeNode> child) {
    if (is_leaf_node_) {
      assert(("This node is the leaf node.", false));
      return;
    }

    children_.push_back(std::move(child));
  }

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

  virtual void Generate(Generator* generator) const;

  constexpr int Size() const { return end_ - start_; }
  constexpr int Start() const { return start_; }
  constexpr int End() const { return end_; }

  virtual ~ParseTreeNode() = default;

 protected:
  // Given index, return the TreeNode that contains the index in its span. If
  // there is no such, then this returns a nullptr.
  ParseTreeNode* GetNext(int index) const;

  // For the elements that are not part of the child nodes, it runs the
  // default_action(g, index); Otherwise, it just calls the Generate of the
  // child node.
  void GenerateWithDefaultAction(
      Generator* generator,
      std::function<void(Generator*, int index)> default_action) const;

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

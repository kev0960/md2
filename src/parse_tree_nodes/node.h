#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "../generators/generator.h"

namespace md2 {

class ParseTreeNode {
 public:
  enum NodeType { NODE, PARAGRAPH, TEXT, BOLD, ITALIC, ESCAPE, LINK, IMAGE };

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
  std::unique_ptr<ParseTreeNode> PopChildrenAt(int index);

  virtual void Generate(Generator* generator) const;

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

 protected:
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

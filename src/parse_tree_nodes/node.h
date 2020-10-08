#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

#include <list>
#include <memory>
#include <vector>

namespace md2 {

class ParseTreeNode {
 public:
  enum NodeType { NODE, PARAGRAPH };

  // Actions to take by generators.
  enum NodeActions { EMIT_CHAR, IGNORE, EMIT_P_START, EMIT_P_END };

  ParseTreeNode(ParseTreeNode* parent, int start)
      : parent_(parent), start_(start) {}
  ParseTreeNode(ParseTreeNode* parent, int start, int end)
      : parent_(parent), start_(start), end_(end) {}

  virtual NodeType GetNodeType() { return NODE; }
  void SetEnd(int end) { end_ = end; }

  void AddChildren(std::unique_ptr<ParseTreeNode> child) {
    children_.push_back(std::move(child));
  }

  ParseTreeNode* GetParent() { return parent_; }
  ParseTreeNode* GetLastChildren() {
    if (children_.empty()) {
      return nullptr;
    }

    return children_.back().get();
  }

  // Given index, return the TreeNode that contains the index in its span. If
  // there is no such, then this returns a nullptr.
  ParseTreeNode* GetNext(int index) const;

  // Set the action per indexes. Returns the index to continue.
  virtual int SetActions(
      std::vector<std::list<ParseTreeNode::NodeActions>>& actions) const;

  constexpr int Size() { return end_ - start_; }
  constexpr int Start() { return start_; }
  constexpr int End() { return end_; }

  virtual ~ParseTreeNode() = default;

 protected:
  std::vector<std::unique_ptr<ParseTreeNode>> children_;

  ParseTreeNode* parent_;

  // Spans [start_, end) in the text. Note that end_ is not included.
  int start_;
  int end_;
};

}  // namespace md2

#endif

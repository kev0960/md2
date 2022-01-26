#include <utility>
#include <vector>

#include "../src/parse_tree.h"
#include "../src/parse_tree_nodes/node.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace md2 {

// Representing the data fields of parse tree node.
struct ParseTreeElement {
  ParseTreeNode::NodeType node_type;
  int start;
  int end;

  // Depth in the parse tree.
  int depth;
};

class ParseTreeComparer {
 public:
  ParseTreeComparer(const std::vector<ParseTreeElement>& tree) : tree_(tree) {}

  void Compare(const ParseTree& tree) const {
    // Conduct the DFS visit of the tree.
    int current = DoCompare(*tree.GetRoot(), 0, 0);
    EXPECT_EQ(current, tree_.size());
  }

 private:
  int DoCompare(const ParseTreeNode& node, int current, int depth) const {
    EXPECT_EQ(tree_[current].node_type, node.GetNodeType())
        << "Error at " << current;
    EXPECT_EQ(tree_[current].start, node.Start()) << "Error at " << current;
    EXPECT_EQ(tree_[current].end, node.End()) << "Error at " << current;
    EXPECT_EQ(tree_[current].depth, depth) << "Error at " << current;

    current++;
    for (const auto& child : node.GetChildren()) {
      current = DoCompare(*child, current, depth + 1);
    }

    return current;
  }

  std::vector<ParseTreeElement> tree_;
};

class NodeMatcher {
 public:
  explicit NodeMatcher(ParseTreeNode::NodeType matching_type)
      : matching_type_(matching_type) {}

  std::vector<const ParseTreeNode*> FetchMatchingNodes(const ParseTree& tree) {
    FindAllMatches(*tree.GetRoot());

    return matched_nodes_;
  }

 private:
  void FindAllMatches(const ParseTreeNode& node) {
    if (node.GetNodeType() == matching_type_) {
      matched_nodes_.push_back(&node);
    }

    for (const auto& child : node.GetChildren()) {
      FindAllMatches(*child);
    }
  }

  ParseTreeNode::NodeType matching_type_;
  std::vector<const ParseTreeNode*> matched_nodes_;
};

}  // namespace md2


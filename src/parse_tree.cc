#include "parse_tree.h"

namespace md2 {

std::vector<std::list<ParseTreeNode::NodeActions>> ParseTree::FlattenTree()
    const {
  std::vector<std::list<ParseTreeNode::NodeActions>> actions;

  // Root node covers the entire string.
  actions.resize(root_->Size());
  root_->SetActions(actions);

  return actions;
}

}  // namespace md2

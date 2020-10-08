#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include <list>
#include <memory>
#include <vector>

#include "parse_tree_nodes/node.h"

namespace md2 {

class ParseTree {
 public:
  ParseTree(std::unique_ptr<ParseTreeNode> root) : root_(std::move(root)) {}

  // Per each character's index, list the actions that need to be done.
  // This will be passed to the Generator to emit the converted result.
  std::vector<std::list<ParseTreeNode::NodeActions>> FlattenTree() const;

 private:
  std::unique_ptr<ParseTreeNode> root_;
};

}  // namespace md2

#endif

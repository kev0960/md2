#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include <list>
#include <memory>
#include <vector>

#include "generators/generator.h"
#include "parse_tree_nodes/node.h"

namespace md2 {

class ParseTree {
 public:
  ParseTree(std::unique_ptr<ParseTreeNode> root) : root_(std::move(root)) {}

  void Generate(Generator* generator);

  const ParseTreeNode* GetRoot() const { return root_.get(); }
  void Print() const { root_->Print(); }

 private:
  std::unique_ptr<ParseTreeNode> root_;
};

}  // namespace md2

#endif

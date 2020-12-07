#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "parse_tree_nodes/node.h"

namespace md2 {

class ParseTree {
 public:
  ParseTree(std::unique_ptr<ParseTreeNode> root,
            std::unordered_map<std::string, ParseTreeNode*>&& refs)
      : root_(std::move(root)), refs_(std::move(refs)) {}

  const ParseTreeNode* GetRoot() const { return root_.get(); }
  void Print() const { root_->Print(); }

  // Returns nullptr if not found.
  ParseTreeNode* FindReferenceNode(std::string_view name) const;

 private:
  std::unique_ptr<ParseTreeNode> root_;

  // References that are identified by the name.
  std::unordered_map<std::string, ParseTreeNode*> refs_;
};

}  // namespace md2

#endif

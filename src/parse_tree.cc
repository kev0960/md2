#include "parse_tree.h"

namespace md2 {

ParseTreeNode* ParseTree::FindReferenceNode(std::string_view name) const {
  if (auto itr = refs_.find(std::string(name)); itr != refs_.end()) {
    return itr->second;
  }

  return nullptr;
}

}  // namespace md2

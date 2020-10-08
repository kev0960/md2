#include "node.h"

namespace md2 {

int ParseTreeNode::SetActions(
    std::vector<std::list<ParseTreeNode::NodeActions>>& actions) const {
  for (const std::unique_ptr<ParseTreeNode>& child : children_) {
    child->SetActions(actions);
  }

  return end_;
}

ParseTreeNode* ParseTreeNode::GetNext(int index) const {
  for (auto& child : children_) {
    if (child->Start() <= index && index < child->End()) {
      return child.get();
    }
  }

  return nullptr;
}

}  // namespace md2

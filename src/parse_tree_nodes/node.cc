#include "node.h"

namespace md2 {

ParseTreeNode* ParseTreeNode::GetNext(int index) const {
  for (auto& child : children_) {
    if (child->Start() <= index && index < child->End()) {
      return child.get();
    }
  }

  return nullptr;
}

void ParseTreeNode::Generate(Generator* generator) const {
  for (const auto& child : children_) {
    child->Generate(generator);
  }
}
void ParseTreeNode ::GenerateWithDefaultAction(
    Generator* generator,
    std::function<void(Generator*, int index)> default_action) const {
  ParseTreeNode* next = nullptr;
  int current = start_;
  while (current != end_) {
    if (next == nullptr) {
      next = GetNext(current);
    }

    // If there is no children next to "current", then just emit every remaining
    // chars.
    if (next == nullptr) {
      while (current != end_) {
        default_action(generator, current++);
      }
    } else {
      // Print until it sees the next child.
      while (current != next->Start()) {
        default_action(generator, current++);
      }

      next->Generate(generator);
      current = next->End();
    }
  }
}

}  // namespace md2

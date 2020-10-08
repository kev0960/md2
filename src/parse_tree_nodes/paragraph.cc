#include "paragraph.h"

namespace md2 {

int ParseTreeParagraphNode::SetActions(
    std::vector<std::list<ParseTreeNode::NodeActions>>& actions) const {
  actions[start_].push_back(EMIT_P_START);

  int current = start_;
  ParseTreeNode* next_node = GetNext(start_);
  while (current < end_) {
    if (next_node == nullptr) {
      // If there is no node after the current index, then no need to check
      // child nodes. Just emit every remaining characters.
      while (current < end_) {
        actions[current].push_back(EMIT_CHAR);
        current++;
      }
      break;
    } else if (current == next_node->Start()) {
      current = SetActions(actions);
      next_node = GetNext(current);
    } else {
      actions[current].push_back(EMIT_CHAR);
      current++;
    }
  }

  actions[end_ - 1].push_back(EMIT_P_END);
  return end_;
}

}  // namespace md2

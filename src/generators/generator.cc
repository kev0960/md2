#include "generator.h"

#include <iostream>

namespace md2 {
namespace {

ParseTreeNode* GetNext(const ParseTreeNode* node, int pos) {
  for (auto& child : node->GetChildren()) {
    if (pos <= child->Start()) {
      return child.get();
    }
  }

  return nullptr;
}

}  // namespace

std::string_view Generator::Generate(const ParseTree& parse_tree) {
  HandleParseTreeNode(*parse_tree.GetRoot());
  return target_;
}

void Generator::GenerateWithDefaultAction(
    const ParseTreeNode& node, std::function<void(int index)> default_action) {
  GenerateWithDefaultActionSpan(node, default_action, node.Start(), node.End());
}

void Generator::GenerateWithDefaultActionSpan(
    const ParseTreeNode& node, std::function<void(int index)> default_action,
    int start, int end) {
  ParseTreeNode* next = nullptr;
  int current = start;
  while (current < end) {
    next = GetNext(&node, current);
    if (next == nullptr) {
      while (current < end) {
        default_action(current++);
      }
    } else {
      // Print until it sees the next child.
      while (current < next->Start()) {
        default_action(current++);
      }
      HandleParseTreeNode(*next);
      current = next->End();
    }
  }
}

std::string_view Generator::GetFileTitle() const {
  const Metadata* metadata = context_->FindMetadataByFilename(filename_);
  if (metadata == nullptr) {
    return "";
  }

  return metadata->GetTitle();
}

}  // namespace md2

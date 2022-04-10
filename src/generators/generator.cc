#include "generator.h"

#include <iostream>

#include "logger.h"

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

std::string_view Generator::Generate() {
  HandleParseTreeNode(*parse_tree_.GetRoot());
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

std::string_view Generator::GetReferenceNodeGeneratedOutput(
    const std::string& ref_name) {
  auto itr = ref_to_generated_.find(ref_name);
  if (itr != ref_to_generated_.end()) {
    return itr->second;
  }

  ParseTreeNode* ref_node = parse_tree_.FindReferenceNode(ref_name);
  if (ref_node == nullptr) {
    return "";
  }

  std::string ref_node_output;
  targets_.push_back(&ref_node_output);

  HandleParseTreeNode(*ref_node->GetChildren()[1]);
  targets_.pop_back();

  ref_to_generated_[ref_name] = std::move(ref_node_output);
  return ref_to_generated_[ref_name];
}

bool Generator::IsInBoxEnvironment(std::string_view box_name) const {
  if (current_box_.empty()) {
    return false;
  }

  return current_box_.back() == box_name;
}

}  // namespace md2

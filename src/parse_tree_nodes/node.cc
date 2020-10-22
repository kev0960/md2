#include "node.h"

#include <iostream>

namespace md2 {

std::unordered_map<ParseTreeNode::NodeType, std::string_view> kNodeTypeToName =
    {{ParseTreeNode::NODE, "NODE"},     {ParseTreeNode::PARAGRAPH, "PARAGRAPH"},
     {ParseTreeNode::TEXT, "TEXT"},     {ParseTreeNode::VERBATIM, "VERBATIM"},
     {ParseTreeNode::BOLD, "BOLD"},     {ParseTreeNode::ITALIC, "ITALIC"},
     {ParseTreeNode::ESCAPE, "ESCAPE"}, {ParseTreeNode::LINK, "LINK"},
     {ParseTreeNode::IMAGE, "IMAGE"},   {ParseTreeNode::HEADER, "HEADER"},
     {ParseTreeNode::BOX, "BOX"},       {ParseTreeNode::TABLE, "TABLE"}};

std::unique_ptr<ParseTreeNode> ParseTreeNode::PopChildrenAt(int index) {
  std::unique_ptr<ParseTreeNode> child = std::move(children_[index]);
  children_.erase(children_.begin() + index);

  return child;
}

ParseTreeNode* ParseTreeNode::GetNext(int pos) const {
  for (auto& child : children_) {
    if (pos <= child->Start()) {
      return child.get();
    }
  }

  return nullptr;
}

int ParseTreeNode::GetNextChildIndex(int pos) const {
  for (int i = 0; i < children_.size(); i++) {
    // Return the first element whose Start() covers current index.
    if (pos <= children_[i]->Start()) {
      return i;
    }
  }

  return children_.size();
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
    next = GetNext(current);

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

void ParseTreeNode::Print(int depth) const {
  std::cout << "Node[" << kNodeTypeToName[GetNodeType()] << "] (" << start_
            << ", " << end_ << "), Depth : " << depth << std::endl;
  for (const auto& child : children_) {
    child->Print(depth + 1);
  }
}

}  // namespace md2

#include "node.h"

#include <iostream>

#include "../logger.h"

namespace md2 {

std::unordered_map<ParseTreeNode::NodeType, std::string_view> kNodeTypeToName =
    {
        {ParseTreeNode::NODE, "NODE"},
        {ParseTreeNode::PARAGRAPH, "PARAGRAPH"},
        {ParseTreeNode::TEXT, "TEXT"},
        {ParseTreeNode::VERBATIM, "VERBATIM"},
        {ParseTreeNode::BOLD, "BOLD"},
        {ParseTreeNode::ITALIC, "ITALIC"},
        {ParseTreeNode::ESCAPE, "ESCAPE"},
        {ParseTreeNode::LINK, "LINK"},
        {ParseTreeNode::IMAGE, "IMAGE"},
        {ParseTreeNode::HEADER, "HEADER"},
        {ParseTreeNode::BOX, "BOX"},
        {ParseTreeNode::TABLE, "TABLE"},
        {ParseTreeNode::LIST, "LIST"},
        {ParseTreeNode::LIST_ITEM, "LIST-ITEM"},
        {ParseTreeNode::ORDERED_LIST, "ORDERED-LIST"},
        {ParseTreeNode::ORDERED_LIST_ITEM, "ORDERED-LIST-ITEM"},
        {ParseTreeNode::COMMAND, "COMMAND"},
        {ParseTreeNode::STRIKE_THROUGH, "STRIKE-THROUGH"},
        {ParseTreeNode::MATH, "MATH"},
        {ParseTreeNode::QUOTE, "QUOTE"},
};

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
  for (size_t i = 0; i < children_.size(); i++) {
    // Return the first element whose Start() covers current index.
    if (pos <= children_[i]->Start()) {
      return i;
    }
  }

  return children_.size();
}

void ParseTreeNode::AddChildBefore(ParseTreeNode* node_to_find,
                                   std::unique_ptr<ParseTreeNode> child) {
  for (size_t i = 0; i < children_.size(); i++) {
    if (children_[i].get() == node_to_find) {
      children_.insert(children_.begin() + i, std::move(child));
      return;
    }
  }
}

void ParseTreeNode::Print(int depth) const {
  LOG(0) << "Node[" << kNodeTypeToName[GetNodeType()] << "] (" << start_ << ", "
         << end_ << "), Depth : " << depth;
  for (const auto& child : children_) {
    child->Print(depth + 1);
  }
}

}  // namespace md2

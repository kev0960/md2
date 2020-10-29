#ifndef PARSE_TREE_COMMAND_H
#define PARSE_TREE_COMMAND_H

#include "node.h"

namespace md2 {

// Node for escaped character. (e.g \*).
class ParseTreeCommandNode : public ParseTreeNode {
 public:
  ParseTreeCommandNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::COMMAND; }
  void SetCommandName(std::string_view command_name) { command_name_ = command_name; }
  std::string_view GetCommandName() const { return command_name_; }

 private:
  std::string command_name_;
};

}  // namespace md2

#endif

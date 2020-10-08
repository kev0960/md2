#ifndef PARSE_TREE_NODES_PARAGRAPH_H
#define PARSE_TREE_NODES_PARAGRAPH_H

#include "../parse_tree.h"

namespace md2 {

// This node represents the single paragraph.
class ParseTreeParagraphNode : public ParseTreeNode {
 public:
  ParseTreeParagraphNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}
  ParseTreeParagraphNode(ParseTreeNode* parent, int start, int end)
      : ParseTreeNode(parent, start, end) {}

  NodeType GetNodeType() override { return ParseTreeNode::PARAGRAPH; }

  int SetActions(std::vector<std::list<ParseTreeNode::NodeActions>>& actions)
      const override;
};

}  // namespace md2

#endif

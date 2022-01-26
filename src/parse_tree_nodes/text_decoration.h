#ifndef PARSE_TREE_NODES_TEXT_DECORATION_H
#define PARSE_TREE_NODES_TEXT_DECORATION_H

#include "../parse_tree.h"

namespace md2 {

class ParseTreeBoldNode : public ParseTreeNode {
 public:
  ParseTreeBoldNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::BOLD; }
};

class ParseTreeItalicNode : public ParseTreeNode {
 public:
  ParseTreeItalicNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::ITALIC; }
};

class ParseTreeStrikeThroughNode : public ParseTreeNode {
 public:
  ParseTreeStrikeThroughNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override {
    return ParseTreeNode::STRIKE_THROUGH;
  }
};

class ParseTreeMathNode : public ParseTreeNode {
 public:
  ParseTreeMathNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::MATH; }
};

class ParseTreeNewlineMathNode : public ParseTreeNode {
 public:
  ParseTreeNewlineMathNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::MATH_NEWLINE; }
};

}  // namespace md2

#endif

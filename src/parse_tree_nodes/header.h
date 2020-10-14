#ifndef PARSE_TREE_NODES_HEADER_H
#define PARSE_TREE_NODES_HEADER_H

#include "../parse_tree.h"

namespace md2 {

class ParseTreeHeaderNode : public ParseTreeNode {
 public:
  ParseTreeHeaderNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::HEADER; }

  void Generate(Generator* generator) const override;
  void SetHeader(std::string_view header);

 private:
  enum HeaderTypes {
    NORMAL_HEADER,
    FANCY_HEADER_FOR_REF,
    LECTURE_HEADER,
    TEMPLATE
  };

  HeaderTypes header_types_;
  std::string_view header_;
};

}  // namespace md2

#endif

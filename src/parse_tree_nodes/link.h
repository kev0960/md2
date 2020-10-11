#ifndef PARSE_TREE_LINK_H
#define PARSE_TREE_LINK_H

#include "node.h"

namespace md2 {

// Node for escaped character. (e.g \*).
class ParseTreeLinkNode : public ParseTreeNode {
 public:
  ParseTreeLinkNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::LINK; }
  void Generate(Generator* generator) const override;

  ParseTreeNode* CreateLinkDesc(int start) {
    link_desc_ = std::make_unique<ParseTreeNode>(nullptr, start);
    return link_desc_.get();
  }

  ParseTreeNode* CreateLink(int start) {
    link_ = std::make_unique<ParseTreeNode>(nullptr, start);
    return link_.get();
  }

  void SetLinkDescEndAndLinkEnd(int link_desc_end, int link_end) {
    link_desc_->SetEnd(link_desc_end);
    link_->SetEnd(link_end);
  }

 private:
  // Description of the link [...] part.
  std::unique_ptr<ParseTreeNode> link_desc_;

  // Actual link (...) part.
  std::unique_ptr<ParseTreeNode> link_;
};

}  // namespace md2

#endif

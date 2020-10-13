#ifndef PARSE_TREE_IMAGE_H
#define PARSE_TREE_IMAGE_H

#include "node.h"

namespace md2 {

class ParseTreeImageNode : public ParseTreeNode {
 public:
  ParseTreeImageNode(ParseTreeNode* parent, int start)
      : ParseTreeNode(parent, start) {}

  NodeType GetNodeType() const override { return ParseTreeNode::IMAGE; }
  void Generate(Generator* generator) const override;

  void SetKeywordNodes(
      std::unordered_map<std::string, std::unique_ptr<ParseTreeNode>>&
          nodes_per_keyword);

 private:
  // Parse the metadata in the image description.
  void ParseImageDescriptionMetadata();

  // Map of image description keyword to the corresponding node's index in the
  // Image description's children vector.
  std::unordered_map<std::string, int> keyword_to_index_;
};

}  // namespace md2

#endif

#include "image.h"

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace md2 {
namespace {}  // namespace

void ParseTreeImageNode::SetKeywordNodes(
    std::unordered_map<std::string, std::unique_ptr<ParseTreeNode>>&
        nodes_per_keyword) {
  // Image description node.
  ParseTreeNode* desc_node = children_[0].get();
  MD2_ASSERT(desc_node->GetNodeType() == ParseTreeNode::NODE, "");

  ParseTreeNode* desc = desc_node->GetChildren()[0].get();
  MD2_ASSERT(desc->GetNodeType() == ParseTreeNode::TEXT, "");

  std::vector<std::pair<std::unique_ptr<ParseTreeNode>, std::string>>
      nodes_and_keyword;

  for (auto& [keyword, keyword_node] : nodes_per_keyword) {
    nodes_and_keyword.push_back(
        std::make_pair(std::move(keyword_node), keyword));
  }

  // Now sort keyword nodes by the starting index.
  std::sort(nodes_and_keyword.begin(), nodes_and_keyword.end(),
            [](const auto& left, const auto& right) {
              return left.first->Start() < right.first->Start();
            });

  for (auto& [keyword_node, keyword] : nodes_and_keyword) {
    desc->AddChildren(std::move(keyword_node));
    keyword_to_index_[keyword] = desc->GetChildren().size() - 1;
  }
}

}  // namespace md2

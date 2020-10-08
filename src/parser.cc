#include "parser.h"

#include "parse_tree_nodes/paragraph.h"

namespace md2 {

ParseTree Parser::GenerateParseTree(std::string_view content) {
  auto root =
      std::make_unique<ParseTreeNode>(/*parent=*/nullptr, 0, content.length());

  ParseTreeNode* current_node = root.get();
  for (size_t i = 0; i != content.size(); i++) {
    // Check if content[i] is a special character.

    if (content.substr(i, 2) == "\n\n") {
      // Mark it as the end of the paragraph.
      current_node->SetEnd(i);
      current_node = current_node->GetParent();

      // Skip the second \n.
      i++;
    } else if (current_node->GetNodeType() != ParseTreeNode::PARAGRAPH) {
      auto p_node = std::make_unique<ParseTreeParagraphNode>(current_node, i);
      current_node->AddChildren(std::move(p_node));
      current_node = current_node->GetLastChildren();
    }
  }

  // Walk up the node and mark its end.
  while (current_node != nullptr) {
    current_node->SetEnd(content.size());
    current_node = current_node->GetParent();
  }

  return ParseTree(std::move(root));
}

}  // namespace md2

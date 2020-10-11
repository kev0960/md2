#include "parser.h"

#include "parse_tree_nodes/paragraph.h"
#include "parse_tree_nodes/text_decoration.h"

namespace md2 {
namespace {

// Create a new node that starts at the given parameter.
template <typename NewNodeType>
ParseTreeNode* CreateNewNode(ParseTreeNode* current_node, size_t start) {
  auto p_node = std::make_unique<NewNodeType>(current_node, start);
  current_node->AddChildren(std::move(p_node));
  current_node = current_node->GetLastChildren();

  return current_node;
}

// Find the parent which is node_type.
// Returns nullptr if there is no such.
ParseTreeNode* FindParent(ParseTreeNode* current_node,
                          ParseTreeNode::NodeType node_type) {
  while (current_node) {
    if (current_node->GetNodeType() == node_type) {
      return current_node;
    }
    current_node = current_node->GetParent();
  }

  return nullptr;
}

// Set the End of the node from the current node til right before root node.
// Returns root node.
ParseTreeNode* MarkEndAllTheWayUp(ParseTreeNode* current_node, int index) {
  while (current_node->GetParent() != nullptr) {
    current_node->SetEnd(index);
    current_node = current_node->GetParent();
  }

  return current_node;
}

}  // namespace

// Rule of parsing.
ParseTree Parser::GenerateParseTree(std::string_view content) {
  auto root = std::make_unique<ParseTreeNode>(/*parent=*/nullptr, 0);

  ParseTreeNode* current_node = root.get();
  int index = 0;
  while (index != content.size()) {
    // If current node is the root node, then create the Paragraph node as a
    // default.
    if (current_node->GetParent() == nullptr) {
      current_node = CreateNewNode<ParseTreeParagraphNode>(current_node, index);
    }

    // We should skip the escape character.
    if (content[index] == '\\') {
    }

    if (content[index] == '*') {
      // "**" always has a higher priority over "*" if
      //   1. Current node is not italic OR
      //   2. Current node does not have BOLD node as a parent.
      //
      //   (1) is needed to handle case like **b*c**d***
      //   Where "**" between "c" and "d" must be treated as *c* and *d*.
      //   (2) is needed to handle case like *a**b***.
      //   Where "**" between "a" and "b" must be treated as opening ** though
      //   the current node is italic.
      if (content.substr(index, 2) == "**" &&
          (current_node->GetNodeType() != ParseTreeNode::ITALIC ||
           FindParent(current_node, ParseTreeNode::BOLD) == nullptr)) {
        if (current_node->GetNodeType() == ParseTreeNode::BOLD) {
          current_node->SetEnd(index + 2);
          current_node = current_node->GetParent();
        } else {
          current_node = CreateNewNode<ParseTreeBoldNode>(current_node, index);
        }

        // Skip the next two **s.
        index += 2;
        continue;
      }

      if (current_node->GetNodeType() == ParseTreeNode::ITALIC) {
        current_node->SetEnd(index + 1);
        current_node = current_node->GetParent();
      } else {
        current_node = CreateNewNode<ParseTreeItalicNode>(current_node, index);
      }

      index += 1;
      continue;
    }

    if (content.substr(index, 2) == "\n\n") {
      current_node = MarkEndAllTheWayUp(current_node, index);

      // Skip the next two \n s.
      index += 2;
      continue;
    }

    index += 1;
  }

  // Walk up the node and mark its end.
  MarkEndAllTheWayUp(current_node, content.size());
  root->SetEnd(content.size());

  return ParseTree(std::move(root));
}

}  // namespace md2

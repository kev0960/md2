#include "parser.h"

#include <unordered_set>

#include "parse_tree_nodes/escape.h"
#include "parse_tree_nodes/link.h"
#include "parse_tree_nodes/paragraph.h"
#include "parse_tree_nodes/text_decoration.h"

namespace md2 {
namespace {

static std::unordered_set<char> kEscapeableChars = {'*', '`', '\\'};

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

ParseTree Parser::GenerateParseTree(std::string_view content) {
  auto root = std::make_unique<ParseTreeNode>(/*parent=*/nullptr, 0);
  GenericParser(content, 0, /*end_parsing_token=*/"", root.get());

  return ParseTree(std::move(root));
}

int Parser::GenericParser(std::string_view content, int start,
                          std::string_view end_parsing_token,
                          ParseTreeNode* root) {
  ParseTreeNode* current_node = root;

  int index = start;
  while (index != content.size()) {
    // If current node is the root node, then create the Paragraph node as a
    // default.
    if (current_node->GetParent() == nullptr) {
      current_node = CreateNewNode<ParseTreeParagraphNode>(current_node, index);
    }

    // We should skip the escape character.
    if (content[index] == '\\') {
      if (index + 1 < content.size()) {
        char c = content[index + 1];
        if (kEscapeableChars.count(c)) {
          current_node =
              CreateNewNode<ParseTreeEscapeNode>(current_node, index);
          current_node->SetEnd(index + 2);
          current_node = current_node->GetParent();

          index += 2;
          continue;
        }
      }
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

    if (content[index] == '[') {
      auto maybe_link = MaybeParseLink(content, index, index);

      if (maybe_link) {
        current_node->AddChildren(std::move(maybe_link));
        continue;
      }
    }

    if (content.substr(index, 2) == "\n\n") {
      current_node = MarkEndAllTheWayUp(current_node, index);

      // Skip the next two \n s.
      index += 2;
      continue;
    }

    // End parsing token must be checked at the paragraph level.
    if (!end_parsing_token.empty() &&
        current_node->GetNodeType() == ParseTreeNode::PARAGRAPH &&
        content.substr(index, end_parsing_token.size()) == end_parsing_token) {
      // Walk up the node and mark its end.
      MarkEndAllTheWayUp(current_node, index + end_parsing_token.size());
      root->SetEnd(index + end_parsing_token.size());
      return index + end_parsing_token.size();
    }

    index += 1;
  }

  // Walk up the node and mark its end.
  MarkEndAllTheWayUp(current_node, content.size());
  root->SetEnd(content.size());

  return content.size();
}

std::unique_ptr<ParseTreeNode> Parser::MaybeParseLink(std::string_view content,
                                                      int start, int& end) {
  auto root = std::make_unique<ParseTreeLinkNode>(nullptr, start);
  int link_desc_end =
      GenericParser(content, start + 1, "](", root->CreateLinkDesc(start));

  if (content.substr(link_desc_end - 2, 2) != "](") {
    return nullptr;
  }

  int link_end = GenericParser(content, link_desc_end, ")",
                               root->CreateLink(link_desc_end));
  if (content.substr(link_end - 1, 1) != ")") {
    return nullptr;
  }

  end = link_end;
  root->SetLinkDescEndAndLinkEnd(link_desc_end, link_end);
  root->SetEnd(link_end);

  return root;
}

}  // namespace md2

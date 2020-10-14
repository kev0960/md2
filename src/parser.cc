#include "parser.h"

#include <iostream>
#include <unordered_set>

#include "parse_tree_nodes/escape.h"
#include "parse_tree_nodes/header.h"
#include "parse_tree_nodes/image.h"
#include "parse_tree_nodes/link.h"
#include "parse_tree_nodes/paragraph.h"
#include "parse_tree_nodes/text_decoration.h"

namespace md2 {
namespace {

static std::unordered_set<char> kEscapeableChars = {'*', '`', '\\'};

// "alt" is not here because alt text is just a default when no xxx= is
// specified.
static std::unordered_set<std::string> kImageDescKeywords = {"caption", "size"};

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

std::optional<std::tuple<std::string, int>> FindDescKeywordPost(
    std::string_view data) {
  size_t current = 0;
  auto found = data.find("=", current);
  while (found != std::string_view::npos) {
    for (const auto& keyword : kImageDescKeywords) {
      if (data.substr(found - keyword.size(), keyword.size()) == keyword) {
        return std::make_tuple(keyword, found - keyword.size());
      }
    }
    current++;
    found = data.find("=", current);
  }

  return std::nullopt;
}

// Builds the image's keyword to node map from the given string.
// E.g If the given content is "(some alt text) caption=abc"
// Then, current_keyword (=alt) will be set as "(some alt text)" and
// new keyword (=caption) will be added to nodes_per_keyword map and the newly
// created node will be returned.
ParseTreeNode* BuildImageKeywordNodes(
    std::string_view content, int start_in_actual_content,
    ParseTreeNode* current_keyword,
    std::unordered_map<std::string, std::unique_ptr<ParseTreeNode>>&
        nodes_per_keyword) {
  std::string_view current_content = content;
  while (!current_content.empty()) {
    auto keyword_and_pos_or = FindDescKeywordPost(current_content);
    if (!keyword_and_pos_or) {
      // There is no keywword (xxx=) inside of the current string.
      return current_keyword;
    }

    auto [keyword, start_pos] = *keyword_and_pos_or;
    current_keyword->SetEnd(start_in_actual_content + start_pos);

    // The xxxx= part is not included in the text node.
    const size_t offset_in_current_content = start_pos + keyword.size() + 1;
    nodes_per_keyword[keyword] = std::make_unique<ParseTreeTextNode>(
        nullptr, start_in_actual_content + offset_in_current_content);
    current_keyword = nodes_per_keyword[keyword].get();

    current_content = current_content.substr(offset_in_current_content);
    start_in_actual_content += offset_in_current_content;
  }

  return current_keyword;
}

// As a default, most of the node will have Paragraph node as a parent node. But
// in some cases like Header, it should be placed Outside of the paragraph node.
//
// Note that exising paragraph node will be placed *AFTER* added node.
ParseTreeNode* HoistNodeAboveParagraph(ParseTreeNode* current_node,
                                       std::unique_ptr<ParseTreeNode> node) {
  if (current_node->Start() == node->Start()) {
    current_node->SetStart(node->End());
    current_node->GetParent()->AddChildrenFront(std::move(node));
    return current_node;
  }

  // If current node is not empty, then we end the paragraph right before
  // the added node. Then create a new paragraph node.
  current_node->SetEnd(node->Start());

  ParseTreeNode* parent = current_node->GetParent();
  int new_paragraph_node_start = node->End();
  parent->AddChildren(std::move(node));

  // Create new paragraph node.
  parent->AddChildren(std::make_unique<ParseTreeParagraphNode>(
      parent, new_paragraph_node_start));
  return current_node->GetParent()->GetLastChildren();
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
      auto maybe_link =
          MaybeParseLink<ParseTreeLinkNode>(content, index, index);

      if (maybe_link) {
        current_node->AddChildren(std::move(maybe_link));
        continue;
      }
    }

    if (content.substr(index, 2) == "![") {
      // We need to pass the position of '['.
      auto maybe_image =
          MaybeParseLink<ParseTreeImageNode>(content, index + 1, index);

      if (maybe_image) {
        ParseTreeImageNode* image =
            static_cast<ParseTreeImageNode*>(maybe_image.get());
        ParseImageDescriptionMetadata(content, image);
        current_node->AddChildren(std::move(maybe_image));
        continue;
      }
    }

    if (content[index] == '#') {
      auto maybe_header = MaybeParseHeader(content, current_node, index, index);
      if (maybe_header) {
        if (current_node->GetNodeType() == ParseTreeNode::PARAGRAPH) {
          current_node =
              HoistNodeAboveParagraph(current_node, std::move(maybe_header));
          continue;
        } else {
          // TODO Mark as a syntax error in the MD file.
          current_node->AddChildren(std::move(maybe_header));
        }
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
      // NOTE that all the child nodes does not include the start of the end
      // parsing token.
      MarkEndAllTheWayUp(current_node, index + end_parsing_token.size() - 1);
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

// Link has the form [link-desc](url)
template <typename LinkNodeType>
std::unique_ptr<ParseTreeNode> Parser::MaybeParseLink(std::string_view content,
                                                      int start, int& end) {
  auto root = std::make_unique<LinkNodeType>(nullptr, start);

  // We should not specify the parent as Link yet (otherwise checking the end
  // parsing token would not work.)
  auto desc = std::make_unique<ParseTreeNode>(nullptr, start);

  // Note that parsing starts after "[" (to prevent infinite loop).
  int desc_end = GenericParser(content, start + 1, "]", desc.get());

  if (content.substr(desc_end - 1, 2) != "](") {
    return nullptr;
  }
  desc->SetEnd(desc_end);

  auto url = std::make_unique<ParseTreeNode>(nullptr, desc_end);

  // Parsing starts after "(".
  int url_end = GenericParser(content, desc_end + 1, ")", url.get());
  if (content.substr(url_end - 1, 1) != ")") {
    return nullptr;
  }
  url->SetEnd(url_end);

  end = url_end;
  root->SetEnd(url_end);

  desc->SetParent(root.get());
  root->AddChildren(std::move(desc));

  url->SetParent(root.get());
  root->AddChildren(std::move(url));

  return root;
}

void Parser::ParseImageDescriptionMetadata(std::string_view content,
                                           ParseTreeImageNode* image) {
  // Keys can be "alt", "caption" or "size".
  std::unordered_map<std::string, std::unique_ptr<ParseTreeNode>>
      nodes_per_keyword;

  // Go through the strings in the paragraph that are not part of the any
  // child node. If it contains xxx= kind of the form, check what xxx is.
  ParseTreeNode* desc_node = image->GetChildren()[0].get();
  assert(desc_node->GetNodeType() == ParseTreeNode::NODE);

  ParseTreeNode* desc = desc_node->GetChildren()[0].get();
  assert(desc->GetNodeType() == ParseTreeNode::PARAGRAPH);

  nodes_per_keyword["alt"] =
      std::make_unique<ParseTreeTextNode>(nullptr, desc->Start());
  ParseTreeNode* current_keyword = nodes_per_keyword["alt"].get();

  // Start of the current segment.
  int start = desc->Start();
  while (start < desc->End()) {
    int next_child_index = desc->GetNextChildIndex(start);

    if (next_child_index == desc->GetChildren().size()) {
      // There is no child node from [start ~ ].  Then [start ~ desc->End())
      // forms the string that is not part of the child node.
      std::string_view raw_str = content.substr(start, desc->End() - start);
      current_keyword = BuildImageKeywordNodes(raw_str, start, current_keyword,
                                               nodes_per_keyword);
      current_keyword->SetEnd(desc->End());
      break;
    } else {
      ParseTreeNode* next_child = desc->GetChildren()[next_child_index].get();

      // Then [start ~ next_child->Start()) forms the raw string.
      std::string_view raw_str =
          content.substr(start, next_child->Start() - start);
      current_keyword = BuildImageKeywordNodes(raw_str, start, current_keyword,
                                               nodes_per_keyword);

      std::unique_ptr<ParseTreeNode> child =
          desc->PopChildrenAt(next_child_index);
      current_keyword->AddChildren(std::move(child));

      start = next_child->End();
    }
  }
  current_keyword->SetEnd(desc->End());

  image->SetKeywordNodes(nodes_per_keyword);
}

std::unique_ptr<ParseTreeNode> Parser::MaybeParseHeader(
    std::string_view content, ParseTreeNode* parent, int start, int& end) {
  // Header must be start from the new line.
  if (start != 0 && content[start - 1] != '\n') {
    return nullptr;
  }

  // Check that the header token only contains # and @ until it sees the space.
  int header_token_end = start;
  while (header_token_end != content.size()) {
    if (content[header_token_end] == ' ') {
      break;
    }

    if (content[header_token_end] != '#' && content[header_token_end] != '@') {
      return nullptr;
    }

    header_token_end++;
  }

  // Now find the end of the header.
  end = header_token_end;
  while (end != content.size() && content[end] != '\n') {
    end++;
  }

  auto header = std::make_unique<ParseTreeHeaderNode>(parent, start);
  header->AddChildren(std::make_unique<ParseTreeTextNode>(header.get(), start));
  header->GetLastChildren()->SetEnd(header_token_end);

  header->AddChildren(
      std::make_unique<ParseTreeTextNode>(header.get(), header_token_end));
  header->GetLastChildren()->SetEnd(end);

  header->SetHeader(content.substr(start, header_token_end - start));
  header->SetEnd(end);

  return header;
}

}  // namespace md2

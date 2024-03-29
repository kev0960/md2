#include "parser.h"

#include <cctype>
#include <unordered_set>
#include <optional>

#include "logger.h"
#include "parse_tree_nodes/box.h"
#include "parse_tree_nodes/command.h"
#include "parse_tree_nodes/escape.h"
#include "parse_tree_nodes/header.h"
#include "parse_tree_nodes/image.h"
#include "parse_tree_nodes/link.h"
#include "parse_tree_nodes/list.h"
#include "parse_tree_nodes/paragraph.h"
#include "parse_tree_nodes/quote.h"
#include "parse_tree_nodes/table.h"
#include "parse_tree_nodes/text_decoration.h"
#include "parse_tree_nodes/verbatim.h"

namespace md2 {
namespace {

static std::unordered_set<char> kEscapeableChars = {'*', '`', '\\',
                                                    '|', '{', '}'};

// Box names that should be always treated as verbatim (nested not allowed).
static std::unordered_set<std::string> kVerbatimBoxNames = {
    "cpp",           "py",        "asm",
    "cpp-formatted", "embed",     "compiler-warning",
    "info",          "info-verb", "info-term",
    "info-format",   "exec",      "objdump",
    "rust"};

// "alt" is not here because alt text is just a default when no xxx= is
// specified.
static std::unordered_set<std::string> kImageDescKeywords = {"caption", "size"};

static std::unordered_map<std::string, int> kCommandToNumArgs = {
    {"sidenote", 1}, {"sc", 1},        {"newline", 1},  {"serif", 1},
    {"htmlonly", 1}, {"latexonly", 1}, {"footnote", 1}, {"esc", 1},
    {"tooltip", 2},  {"ref", 1}};

// Commands that take verbatim stuff in the argument (should not parse whatever
// that's inside {}) and the index of those args.
static std::unordered_map<std::string, std::unordered_set<size_t>>
    kVerbatimCommands = {{"htmlonly", {0}}, {"tooltip", {1}}, {"ref", {0}}};

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
    current_node->GetParent()->AddChildBefore(current_node, std::move(node));
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

// start is pointing the beginning of the list.
// (e.g position of '*' for unordered lists, or the position of '1' for the
// ordered lists)
//
// Returns the list depth and the end of the list header.
// E.g
//
//        1. abc
// |<---->|  ^
//   depth   end of list header
//
std::optional<std::pair<int, int>> IsCorrectListHeader(std::string_view content,
                                                       int start) {
  int list_depth = 0;

  int current = start - 1;
  while (current >= 0) {
    if (content[current] == ' ') {
      list_depth++;
      current--;
    } else if (content[current] == '\n') {
      break;
    } else {
      return std::nullopt;
    }
  }

  if (content.substr(start, 2) == "* ") {
    return std::make_pair(list_depth, start + 2);
  } else if (std::isdigit(content[start]) &&
             content.substr(start + 1, 2) == ". ") {
    return std::make_pair(list_depth, start + 3);
  }

  return std::nullopt;
}

// Check whether some sentence is the start of the list. To be the start of the
// list, it must start the sentence with optional whitespaces and followed by
// '*' and ' '. For example,
//    * abc <-- Start of the list.
// *a <-- Not start of the list.
//
// start must be the start of the line (right after newline).
//
// If found, this will return the list depth (number of ' ' before *) and list
// start (position of '*').
std::optional<std::pair<int, int>> IsStartOfTheList(std::string_view content,
                                                    int start) {
  int list_depth = 0;
  while (start < static_cast<int>(content.size())) {
    if (content[start] == ' ') {
      list_depth++;
      start++;
    } else if (content[start] == '*') {
      if (content.substr(start, 2) != "* ") {
        return std::nullopt;
      } else {
        return std::make_pair(list_depth, start);
      }
    } else if (std::isdigit(content[start]) &&
               content.substr(start + 1, 2) == ". ") {
      return std::make_pair(list_depth, start);
    } else {
      return std::nullopt;
    }
  }

  return std::nullopt;
}

bool IsListItemType(const ParseTreeNode* node) {
  return node->GetNodeType() == ParseTreeNode::LIST_ITEM ||
         node->GetNodeType() == ParseTreeNode::ORDERED_LIST_ITEM;
}

int GetDepth(std::unique_ptr<ParseTreeNode>& child) {
  MD2_ASSERT(IsListItemType(child.get()), "");

  return static_cast<ParseTreeListItemNode*>(child.get())->GetListDepth();
}

// Construct list from list items in range [start, end). Note that end is not
// included.
//
// **children** will be moved away.
std::unique_ptr<ParseTreeListNode> ConstructListFromListItems(
    std::vector<std::unique_ptr<ParseTreeNode>>& children, int start, int end) {
  // Pair of list node and the corresponding depth.
  std::vector<std::pair<ParseTreeListNode*, int>> current_lists;

  std::unique_ptr<ParseTreeListNode> top_level_list;

  int list_end = children[end - 1]->End();

  int current = start;
  while (current != end) {
    if (current_lists.empty() ||
        current_lists.back().second < GetDepth(children[current])) {
      bool is_ordered =
          children[current]->GetNodeType() == ParseTreeNode::ORDERED_LIST_ITEM;
      auto list = std::make_unique<ParseTreeListNode>(
          nullptr, children[current]->Start(), is_ordered);
      current_lists.push_back(
          std::make_pair(list.get(), GetDepth(children[current])));

      // Make current list item as the children of the list.
      current_lists.back().first->AddChildren(std::move(children[current]));

      if (current_lists.size() == 1) {
        top_level_list = std::move(list);
      } else {
        list->SetParent(current_lists[current_lists.size() - 2].first);
        current_lists[current_lists.size() - 2].first->AddChildren(
            std::move(list));
      }
    } else if (current_lists.back().second == GetDepth(children[current])) {
      // Then we can just append the current list item to the list node.
      current_lists.back().first->AddChildren(std::move(children[current]));
    } else if (current_lists.back().second > GetDepth(children[current])) {
      int depth = GetDepth(children[current]);
      while (!current_lists.empty()) {
        if (current_lists.back().second <= depth) {
          break;
        }

        // Otherwise mark the end of the list and pop.
        current_lists.back().first->SetEnd(children[current]->Start());
        current_lists.back().first->SetListItemIndexes();
        current_lists.pop_back();
      }

      // Oops something went wrong.
      if (current_lists.empty()) {
        return nullptr;
      }

      if (current_lists.back().second == depth) {
        current_lists.back().first->AddChildren(std::move(children[current]));
      } else {
        bool is_ordered = children[current]->GetNodeType() ==
                          ParseTreeNode::ORDERED_LIST_ITEM;
        auto list = std::make_unique<ParseTreeListNode>(
            nullptr, children[current]->Start(), is_ordered);
        current_lists.push_back(
            std::make_pair(list.get(), GetDepth(children[current])));

        // Make current list item as the children of the list.
        current_lists.back().first->AddChildren(std::move(children[current]));
        list->SetParent(current_lists[current_lists.size() - 2].first);
        current_lists[current_lists.size() - 2].first->AddChildren(
            std::move(list));
      }
    }

    current++;
  }

  for (auto [current_list, depth] : current_lists) {
    current_list->SetEnd(list_end);
    current_list->SetListItemIndexes();
  }

  return top_level_list;
}

// Returns the right after the end of the empty line if exists.
std::optional<int> IsEmptyLine(std::string_view content, int start) {
  while (start < static_cast<int>(content.size())) {
    if (content[start] == ' ') {
      start++;
    } else if (content[start] == '\n') {
      return start + 1;
    } else {
      return std::nullopt;
    }
  }

  return content.size();
}

bool IsEndBoxToken(std::string_view content, size_t index) {
  if (index < 1) {
    return false;
  }

  if (content.substr(index - 1, 5) == "\n```\n") {
    return true;
  }

  if (content.substr(index - 1, 4) == "\n```" && index + 3 == content.size()) {
    return true;
  }

  return false;
}

}  // namespace

ParseTree Parser::GenerateParseTree(std::string_view content) {
  auto root = std::make_unique<ParseTreeNode>(/*parent=*/nullptr, 0);
  RefContainer refs;

  GenericParser(content, 0, /*end_parsing_token=*/"", root.get(), &refs);

  PostProcessList(root.get());
  return ParseTree(std::move(root), std::move(refs));
}

size_t Parser::GenericParser(std::string_view content, size_t start,
                             std::string_view end_parsing_token,
                             ParseTreeNode* root, RefContainer* refs,
                             bool use_text, bool must_inline, bool no_link) {
  ParseTreeNode* current_node = root;

  size_t index = start;
  while (index < content.size()) {
    // If current node is the root node, then create the Paragraph node as a
    // default.
    if (current_node->GetParent() == nullptr) {
      if (use_text) {
        current_node = CreateNewNode<ParseTreeTextNode>(current_node, index);
      } else {
        current_node =
            CreateNewNode<ParseTreeParagraphNode>(current_node, index);
      }
    }

    if (must_inline && content[index] == '\n') {
      return start;
    }

    // Handle the escape character or the command.
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
        } else {
          auto maybe_command =
              MaybeParseCommand(content, current_node, refs, index, index);
          if (maybe_command != nullptr) {
            current_node->AddChildren(std::move(maybe_command));
            continue;
          }
        }
      }
    }

    // This can be the start of the ordered list. (e.g "1.")
    if (std::isdigit(content[index]) && index + 2 < content.size() &&
        content.substr(index + 1, 2) == ". ") {
      if (auto maybe_list =
              MaybeParseList(content, current_node, refs, index, index);
          maybe_list != nullptr) {
        if (current_node->GetNodeType() == ParseTreeNode::PARAGRAPH) {
          current_node =
              HoistNodeAboveParagraph(current_node, std::move(maybe_list));
        } else {
          current_node->AddChildren(std::move(maybe_list));
        }

        continue;
      }
    }

    if (content.substr(index, 2) == "~~") {
      if (current_node->GetNodeType() == ParseTreeNode::STRIKE_THROUGH) {
        current_node->SetEnd(index + 2);
        current_node = current_node->GetParent();
      } else {
        current_node =
            CreateNewNode<ParseTreeStrikeThroughNode>(current_node, index);
      }

      index += 2;
      continue;
    }

    if (content.substr(index, 2) == "$$") {
      size_t start = index;
      size_t current = index + 2;
      while (content.substr(current, 2) != "$$" && current < content.size()) {
        current++;
      }

      if (current != content.size()) {
        auto* node = CreateNewNode<ParseTreeMathNode>(current_node, start);
        node->SetEnd(current + 2);
        index = current + 2;
      } else {
        index += 1;
      }
      continue;
    }

    if (content.substr(index, 2) == "\\[") {
      size_t start = index;
      size_t current = index + 2;
      while (content.substr(current, 2) != "\\]" && current < content.size()) {
        current++;
      }

      if (current != content.size()) {
        auto* node =
            CreateNewNode<ParseTreeNewlineMathNode>(current_node, start);
        node->SetEnd(current + 2);
        index = current + 2;
      } else {
        index += 1;
      }
      continue;
    }

    if (content[index] == '*') {
      if (auto maybe_list =
              MaybeParseList(content, current_node, refs, index, index);
          maybe_list != nullptr) {
        if (current_node->GetNodeType() == ParseTreeNode::PARAGRAPH) {
          current_node =
              HoistNodeAboveParagraph(current_node, std::move(maybe_list));
        } else {
          current_node->AddChildren(std::move(maybe_list));
        }

        continue;
      }

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

    if (content[index] == '[' && !no_link) {
      auto maybe_link =
          MaybeParseLink<ParseTreeLinkNode>(content, refs, index, index);

      if (maybe_link) {
        current_node->AddChildren(std::move(maybe_link));
        continue;
      }
    }

    if (content.substr(index, 2) == "![") {
      // We need to pass the position of '['.
      auto maybe_image =
          MaybeParseLink<ParseTreeImageNode>(content, refs, index + 1, index);

      if (maybe_image) {
        ParseTreeImageNode* image =
            static_cast<ParseTreeImageNode*>(maybe_image.get());
        ParseImageDescriptionMetadata(content, image);
        current_node->AddChildren(std::move(maybe_image));
        continue;
      }
    }

    if (content[index] == '#') {
      auto maybe_header =
          MaybeParseHeader(content, current_node, refs, index, index);
      if (maybe_header) {
        if (current_node->GetNodeType() == ParseTreeNode::PARAGRAPH) {
          current_node =
              HoistNodeAboveParagraph(current_node, std::move(maybe_header));
        } else {
          // TODO Mark as a syntax error in the MD file.
          current_node->AddChildren(std::move(maybe_header));
        }
        continue;
      }
    }

    if (content.substr(index, 2) == "> ") {
      if (index == 0 || content[index - 1] == '\n') {
        auto maybe_quote =
            MaybeParseQuote(content, current_node, refs, index, index);
        if (maybe_quote) {
          if (current_node->GetNodeType() == ParseTreeNode::PARAGRAPH) {
            current_node =
                HoistNodeAboveParagraph(current_node, std::move(maybe_quote));
          } else {
            // TODO Mark as a syntax error in the MD file.
            current_node->AddChildren(std::move(maybe_quote));
          }
          continue;
        }
      }
    }

    if (content.substr(index, 3) == "```") {
      if (IsEndBoxToken(content, index)) {
        // Then this is the absolute end token. We do not include the "\n".
        MarkEndAllTheWayUp(current_node, index - 1);
        root->SetEnd(index + 3);
        return index + 3;
      }
    }

    if (content[index] == '`') {
      auto maybe_box = MaybeParseBox(content, current_node, refs, index, index);
      if (maybe_box) {
        if (current_node->GetNodeType() == ParseTreeNode::PARAGRAPH ||
            current_node->GetNodeType() == ParseTreeNode::TEXT) {
          current_node =
              HoistNodeAboveParagraph(current_node, std::move(maybe_box));
        } else {
          current_node->AddChildren(std::move(maybe_box));
        }
        continue;
      } else if (content.substr(index, 3) != "```") {
        int verbatim_start = index;
        index++;

        // Try to find the end `.
        while (index < content.size()) {
          if (content[index] == '`') {
            current_node->AddChildren(std::make_unique<ParseTreeVerbatimNode>(
                current_node, verbatim_start));
            current_node->GetLastChildren()->SetEnd(index + 1);
            break;
          }
          index++;
        }
        index++;
        continue;
      }
    }

    // Note that we should not ignore consecutive \n\n s if the end parsing
    // token is newline (since the goal of the end parsing token is recognizing
    // that).
    if (end_parsing_token != "\n" && content.substr(index, 2) == "\n\n") {
      current_node = MarkEndAllTheWayUp(current_node, index);

      // Skip the next two \n s.
      index += 2;
      continue;
    }

    if (index > 0 && content.substr(index - 1, 2) == "\n|") {
      auto maybe_table =
          MaybeParseTable(content, current_node, refs, index, index);
      if (maybe_table) {
        if (current_node->GetNodeType() == ParseTreeNode::PARAGRAPH) {
          current_node =
              HoistNodeAboveParagraph(current_node, std::move(maybe_table));
        } else {
          current_node->AddChildren(std::move(maybe_table));
        }
        continue;
      }
    }

    // End parsing token must be checked at the paragraph level.
    if (!end_parsing_token.empty() &&
        (current_node->GetNodeType() == ParseTreeNode::PARAGRAPH ||
         (use_text && current_node->GetNodeType() == ParseTreeNode::TEXT)) &&
        content.substr(index, end_parsing_token.size()) == end_parsing_token) {
      // Walk up the node and mark its end.
      // NOTE that all the child nodes does not include the start of the end
      // parsing token.
      MarkEndAllTheWayUp(current_node, index);
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
                                                      RefContainer* refs,
                                                      size_t start,
                                                      size_t& end) {
  LOG(3) << "Maybe link : " << content.substr(start, 10) << " -- " << start;
  int node_start = start;
  if constexpr (std::is_same_v<LinkNodeType, ParseTreeImageNode>) {
    // Need to include the preceding '!'.
    node_start--;
  }

  auto root = std::make_unique<LinkNodeType>(nullptr, node_start);

  // We should not specify the parent as Link yet (otherwise checking the end
  // parsing token would not work.)
  auto desc = std::make_unique<ParseTreeNode>(nullptr, node_start);

  bool no_nested_link = true;

  // TODO Allow nested_link for image when md files are fixed.
  /*
  if constexpr (std::is_same_v<LinkNodeType, ParseTreeLinkNode>) {
    no_nested_link = true;
  }
  */

  // Note that parsing starts after "[" (to prevent infinite loop).
  size_t desc_end =
      GenericParser(content, start + 1, "]", desc.get(), refs,
                    /*use_text=*/true, /*must_inline=*/true, no_nested_link);

  if (start == desc_end || content.substr(desc_end - 1, 2) != "](") {
    return nullptr;
  }
  desc->SetEnd(desc_end);

  auto url = std::make_unique<ParseTreeNode>(nullptr, desc_end);

  // Parsing starts after "(".
  size_t url_end = GenericParser(content, desc_end + 1, ")", url.get(), refs,
                                 /*use_text=*/true, /*must_inline=*/true);
  if (url_end == desc_end + 1 || content.substr(url_end - 1, 1) != ")") {
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
  MD2_ASSERT(desc_node->GetNodeType() == ParseTreeNode::NODE, "");

  ParseTreeNode* desc = desc_node->GetChildren()[0].get();
  MD2_ASSERT(desc->GetNodeType() == ParseTreeNode::TEXT, "");

  nodes_per_keyword["alt"] =
      std::make_unique<ParseTreeTextNode>(nullptr, desc->Start());
  ParseTreeNode* current_keyword = nodes_per_keyword["alt"].get();

  // Start of the current segment.
  int start = desc->Start();
  while (start < desc->End()) {
    int next_child_index = desc->GetNextChildIndex(start);

    if (next_child_index == static_cast<int>(desc->GetChildren().size())) {
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
    std::string_view content, ParseTreeNode* parent, RefContainer* refs,
    size_t start, size_t& end) {
  // Header must be start from the new line.
  if (start != 0 && content[start - 1] != '\n') {
    return nullptr;
  }

  // Check that the header token only contains # and @ until it sees the space.
  int header_token_end = start;
  while (header_token_end != static_cast<int>(content.size())) {
    if (content[header_token_end] == ' ') {
      break;
    }

    if (content[header_token_end] != '#' && content[header_token_end] != '@') {
      return nullptr;
    }

    header_token_end++;
  }

  auto header = std::make_unique<ParseTreeHeaderNode>(parent, start);
  header->AddChildren(std::make_unique<ParseTreeTextNode>(header.get(), start));
  header->GetLastChildren()->SetEnd(header_token_end);

  // If the header token is ###@, then we should parse it.
  // TODO This should be a default behavior. Some old markdown uses non escaped
  // syntaxs in the header which prevents us from using the GenericParser.
  if (content.substr(start, header_token_end - start) == "###@") {
    auto header_content =
        std::make_unique<ParseTreeNode>(nullptr, header_token_end);
    size_t header_end = GenericParser(content, header_token_end, "\n",
                                      header_content.get(), refs,
                                      /*use_text=*/true);
    if (header_end != content.size() && content[header_end - 1] != '\n') {
      return nullptr;
    }

    header_content->SetEnd(header_end);
    header->AddChildren(std::move(header_content));
    header->SetEnd(header_end);
    end = header_end;

    return header;
  }

  // Now find the end of the header.
  end = header_token_end;
  while (end != content.size() && content[end] != '\n') {
    end++;
  }

  header->AddChildren(
      std::make_unique<ParseTreeTextNode>(header.get(), header_token_end));
  header->GetLastChildren()->SetEnd(end);
  header->SetEnd(end);

  return header;
}

std::unique_ptr<ParseTreeNode> Parser::MaybeParseBox(std::string_view content,
                                                     ParseTreeNode* parent,
                                                     RefContainer* refs,
                                                     size_t start,
                                                     size_t& end) {
  LOG(3) << "start : " << start << " : " << content.substr(start, 10);
  if (start != 0 && content[start - 1] != '\n') {
    return nullptr;
  }

  if (content.substr(start, 3) != "```") {
    return nullptr;
  }

  // This is the end of the verbatim mark.
  if (content.substr(start, 4) == "```\n") {
    return nullptr;
  }

  // Try to read the box name.
  size_t box_name_end = start + 3;
  while (content[box_name_end] != '\n' && box_name_end < content.size()) {
    box_name_end++;
  }

  // If the box name cannot be found, then this is the End of the box marker
  // (```)
  if (box_name_end == start + 3) {
    return nullptr;
  }

  std::string box_name(content.substr(start + 3, box_name_end - (start + 3)));
  if (kVerbatimBoxNames.count(box_name)) {
    // Then the nested ```s are not allowed. Just find the end of the box.
    end = box_name_end + 1;
    while (content.substr(end, 3) != "```") {
      end++;

      if (end >= content.size()) {
        end = start;
        return nullptr;
      }
    }

    auto code = std::make_unique<ParseTreeVerbatimNode>(parent, start);

    // Box name becomes the first text child node.
    code->AddChildren(
        std::make_unique<ParseTreeTextNode>(code.get(), start + 3));
    code->GetLastChildren()->SetEnd(box_name_end);

    // The actual code becomes the next text child node.
    code->AddChildren(
        std::make_unique<ParseTreeTextNode>(code.get(), box_name_end + 1));
    code->GetLastChildren()->SetEnd(end);

    code->SetEnd(end + 3);
    end += 3;

    return code;
  }

  // If this is not code, then the box node can be nested.
  auto box = std::make_unique<ParseTreeBoxNode>(parent, start);

  // Box name becomes the first text child node.
  box->AddChildren(std::make_unique<ParseTreeTextNode>(box.get(), start + 3));
  box->GetLastChildren()->SetEnd(box_name_end);

  // We should not specify the parent as Box yet (otherwise checking the end
  // parsing token would not work.) Note that start of the box content does not
  // contain the \n of the box name.
  auto box_content_node =
      std::make_unique<ParseTreeNode>(nullptr, box_name_end + 1);

  bool is_ref_box = (box_name.substr(0, 4) == "ref-" && box_name.size() > 4);
  size_t box_content_end =
      GenericParser(content, box_name_end + 1, "```", box_content_node.get(),
                    refs, /*use_text=*/is_ref_box);

  LOG(3) << "box content end : [" << content.substr(box_content_end - 4, 4)
         << "]";
  if (content.substr(box_content_end - 4, 4) != "\n```") {
    return nullptr;
  }

  box_content_node->SetEnd(box_content_end);
  box_content_node->SetParent(box.get());
  box->AddChildren(std::move(box_content_node));

  // Note that ``` is included in the paragraph of the box content node.
  end = box_content_end;
  box->SetEnd(end);

  if (is_ref_box) {
    // Then register this box node as a reference.
    (*refs)[box_name.substr(4)] = box.get();
  }

  LOG(3) << "End box " << box_name << "!\n";
  return box;
}

std::unique_ptr<ParseTreeNode> Parser::MaybeParseTable(std::string_view content,
                                                       ParseTreeNode* parent,
                                                       RefContainer* refs,
                                                       size_t start,
                                                       size_t& end) {
  // Table must start with newline.
  if (start != 0 && content[start - 1] != '\n') {
    return nullptr;
  }

  if (content[start] != '|') {
    return nullptr;
  }

  auto table = std::make_unique<ParseTreeTableNode>(parent, start);

  size_t current = start + 1;
  while (true) {
    while (true) {
      auto cell = std::make_unique<ParseTreeNode>(nullptr, current);
      size_t cell_end = GenericParser(content, current, "|", cell.get(), refs);

      // Not a valid table.
      if (content[cell_end - 1] != '|') {
        return nullptr;
      }

      cell->SetParent(table.get());
      table->AddChildren(std::move(cell));

      if (cell_end >= content.size()) {
        // Then the current row ends.
        current = cell_end;
        break;
      }

      if (content[cell_end] == '\n') {
        // Then the current row ends.
        table->SetRowSizeIfNotSpecified();
        current = cell_end + 1;
        break;
      }
      current = cell_end;
    }

    if (current >= content.size() || content[current] != '|') {
      break;
    }

    current += 1;
  }

  table->SetEnd(current);

  end = current;
  return table;
}

// Parsing the list is a bit tricky.
//
// First, we identify a list if the newline starts with "* ". Any space that
// comes before * are ignored.
//
// List is continued until it either sees another start of the new list or the
// double newlines. For example,
//
// * abc
// * def
//
// Above two forms a list.
//
//  * abc
//  def   <-- These three are the part of the list item
//  ghi
//  * abc
//
// The first list item will contain "abc\ndef\nghi". The second list item will
// contain "abc".
//
//  * abc <-- End of the list
//
//  def
//  ghi
//
//  * def
//
// The first list item will only contain "abc". Then it will be followed with
// the paragraph. After that there will be a list item "def".
//
// Those list items will from the actual list after the post-processing part in
// the parser.
std::unique_ptr<ParseTreeNode> Parser::MaybeParseList(std::string_view content,
                                                      ParseTreeNode* parent,
                                                      RefContainer* refs,
                                                      size_t start,
                                                      size_t& end) {
  auto list_header_info = IsCorrectListHeader(content, start);
  if (!list_header_info) {
    return nullptr;
  }

  auto [list_depth, current] = *list_header_info;

  bool is_ordered = content[start] != '*';

  // Now try to create the list item element.
  auto list_item = std::make_unique<ParseTreeListItemNode>(
      nullptr, start - list_depth, is_ordered);
  list_item->SetListDepth(list_depth);

  while (true) {
    size_t list_end =
        GenericParser(content, current, "\n", list_item.get(), refs);
    if (list_end == content.size()) {
      list_item->SetEnd(list_end);
      list_item->SetParent(parent);
      end = list_end;
      return list_item;
    }

    if (list_end != content.size() && content[list_end - 1] != '\n') {
      return nullptr;
    }

    // if the empty new line comes right after "\n", then we can think of it as
    // the end of the list.
    if (auto empty_line_end = IsEmptyLine(content, list_end); empty_line_end) {
      // List item will cover the entire empty line, including the newline at
      // the end.
      list_item->SetEnd(*empty_line_end);
      list_item->SetParent(parent);
      end = *empty_line_end;
      return list_item;
    } else if (auto next_list_info = IsStartOfTheList(content, list_end);
               next_list_info.has_value()) {
      // If the next line consists the another set of the new list, then we
      // start a new list.
      list_item->SetEnd(list_end);
      list_item->SetParent(parent);
      end = list_end;
      return list_item;
    } else if (content.substr(list_end, 4) == "```\n") {
      // This is the case like:
      // ```note
      // 1. abc
      // ```
      //
      // Where people can omit the newline after the ordered list when it ends
      // the box.
      list_item->SetEnd(list_end);
      list_item->SetParent(parent);
      end = list_end;
      return list_item;
    }

    // Otherwise, continue parsing.
    current = list_end;
  }

  return list_item;
}

std::unique_ptr<ParseTreeNode> Parser::MaybeParseCommand(
    std::string_view content, ParseTreeNode* parent, RefContainer* refs,
    size_t start, size_t& end) {
  LOG(3) << "Maybe Command : " << content.substr(start, 5) << "--" << start;

  // Commands are in form \command{}{}..{}
  // Number of {} s can be vary.
  std::string_view command_name;
  int num_arg = 0;

  size_t command_start = start + 1;
  for (const auto& [name, arg] : kCommandToNumArgs) {
    if (content.substr(command_start, name.size()) == name &&
        command_start + name.size() < content.size() &&
        content[command_start + name.size()] == '{') {
      command_name = name;
      num_arg = arg;
      break;
    }
  }

  if (command_name.empty()) {
    return nullptr;
  }

  auto command = std::make_unique<ParseTreeCommandNode>(parent, start);
  command->SetCommandName(command_name);

  // current now points {.
  size_t current = command_start + command_name.size();

  std::unordered_set<size_t> verbatim_args;
  if (const auto itr = kVerbatimCommands.find(std::string(command_name));
      itr != kVerbatimCommands.end()) {
    verbatim_args = itr->second;
  }

  size_t arg_index = 0;
  while (num_arg--) {
    if (current >= content.size() || content[current] != '{') {
      return nullptr;
    }

    // Arg does not include {.
    current++;

    // In this case, we should treat current argument as a verbatim. Do not
    // parse whatever that is inside {}.
    if (verbatim_args.count(arg_index)) {
      size_t arg_start = current;
      while (current < content.size()) {
        if (content[current] == '}' && content[current - 1] != '\\') {
          auto arg = std::make_unique<ParseTreeTextNode>(nullptr, arg_start);
          arg->SetEnd(current);

          command->AddChildren(std::move(arg));

          // Current now points after '}'.
          current++;
          break;
        }
        current++;
      }
    } else {
      auto arg = std::make_unique<ParseTreeNode>(nullptr, current);
      current = GenericParser(content, current, "}", arg.get(), refs, true);

      if (content[current - 1] != '}') {
        return nullptr;
      }

      if (num_arg > 0 && content[current] != '{') {
        return nullptr;
      }

      // Arg does not include }.
      arg->SetEnd(current - 1);
      command->AddChildren(std::move(arg));
    }

    arg_index++;
  }

  end = current;
  command->SetEnd(current);
  return command;
}

std::unique_ptr<ParseTreeNode> Parser::MaybeParseQuote(std::string_view content,
                                                       ParseTreeNode* parent,
                                                       RefContainer* refs,
                                                       size_t start,
                                                       size_t& end) {
  if (content.substr(start, 2) != "> ") {
    return nullptr;
  }

  auto quote = std::make_unique<ParseTreeQuoteNode>(nullptr, start);
  size_t current = start + 2;

  while (true) {
    current = GenericParser(content, current, "\n", quote.get(), refs,
                            /*use_text=*/true);
    if (current != content.size() && content[current - 1] != '\n') {
      return nullptr;
    }

    if (current >= content.size() || content.substr(current, 2) != "> ") {
      quote->SetParent(parent);
      quote->SetEnd(current);
      end = current;
      return quote;
    }

    current += 2;
  }

  return quote;
}

void Parser::PostProcessList(ParseTreeNode* root) {
  // Traverse nodes and convert list_items into part of list node based on their
  // list depth.
  std::vector<std::unique_ptr<ParseTreeNode>>& children = root->GetChildren();
  for (size_t current = 0; current != children.size(); current++) {
    // If the list item is found, then locate the end of the list item.
    if (IsListItemType(children[current].get())) {
      size_t list_item_end = current;
      while (list_item_end != children.size()) {
        if (!IsListItemType(children[list_item_end].get())) {
          break;
        }

        list_item_end++;
      }

      auto list_node =
          ConstructListFromListItems(children, current, list_item_end);
      if (list_node != nullptr) {
        // Erase the list item elements (they are already moved out anyway into
        // the child of lists.).
        children.erase(children.begin() + current,
                       children.begin() + list_item_end);
        children.insert(children.begin() + current, std::move(list_node));
      }
    } else {
      PostProcessList(children[current].get());
    }
  }
}

}  // namespace md2

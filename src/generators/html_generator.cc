#include "html_generator.h"

namespace md2 {
namespace {

template <typename To, typename From>
const To& CastNodeTypes(const From& node) {
  return *static_cast<const To*>(&node);
}

}  // namespace

void HTMLGenerator::HandleParseTreeNode(const ParseTreeNode& node) {
  switch (node.GetNodeType()) {
    case ParseTreeNode::NODE:
      for (const auto& child : node.GetChildren()) {
        HandleParseTreeNode(*child);
      }
      break;
    case ParseTreeNode::PARAGRAPH:
      HandleParagraph(CastNodeTypes<ParseTreeParagraphNode>(node));
      break;
    case ParseTreeNode::TEXT:
      HandleText(CastNodeTypes<ParseTreeTextNode>(node));
      break;
    case ParseTreeNode::BOLD:
      HandleBold(CastNodeTypes<ParseTreeBoldNode>(node));
      break;
    case ParseTreeNode::ITALIC:
      HandleItalic(CastNodeTypes<ParseTreeItalicNode>(node));
      break;
    case ParseTreeNode::LINK:
      HandleLink(CastNodeTypes<ParseTreeLinkNode>(node));
      break;
    case ParseTreeNode::IMAGE:
      HandleImage(CastNodeTypes<ParseTreeImageNode>(node));
      break;
    case ParseTreeNode::TABLE:
      HandleTable(CastNodeTypes<ParseTreeTableNode>(node));
      break;
    case ParseTreeNode::ORDERED_LIST:
    case ParseTreeNode::LIST:
      HandleList(CastNodeTypes<ParseTreeListNode>(node));
      break;
    case ParseTreeNode::ORDERED_LIST_ITEM:
    case ParseTreeNode::LIST_ITEM:
      HandleListItem(CastNodeTypes<ParseTreeListItemNode>(node));
      break;
    case ParseTreeNode::HEADER:
      HandleHeader(CastNodeTypes<ParseTreeHeaderNode>(node));
      break;
    case ParseTreeNode::VERBATIM:
      HandleVerbatim(CastNodeTypes<ParseTreeVerbatimNode>(node));
      break;
    case ParseTreeNode::ESCAPE:
      HandleEscape(CastNodeTypes<ParseTreeEscapeNode>(node));
      break;
    case ParseTreeNode::COMMAND:
      HandleCommand(CastNodeTypes<ParseTreeCommandNode>(node));
      break;
    case ParseTreeNode::STRIKE_THROUGH:
      HandleStrikeThrough(CastNodeTypes<ParseTreeStrikeThroughNode>(node));
      break;
    default:
      break;
  }
}

void HTMLGenerator::HandleParagraph(const ParseTreeParagraphNode& node) {
  GetCurrentTarget()->append("<p>");

  GenerateWithDefaultAction(
      node, [this](int index) { GetCurrentTarget()->push_back(md_[index]); });

  GetCurrentTarget()->append("</p>");
}

void HTMLGenerator::HandleText(const ParseTreeTextNode& node) {
  GenerateWithDefaultAction(
      node, [this](int index) { GetCurrentTarget()->push_back(md_[index]); });
}

void HTMLGenerator::HandleBold(const ParseTreeBoldNode& node) {
  GetCurrentTarget()->append("<span class='font-weight-bold'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { GetCurrentTarget()->push_back(md_[index]); },
      node.Start() + 2, node.End() - 2);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleItalic(const ParseTreeItalicNode& node) {
  GetCurrentTarget()->append("<span class='font-italic'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { GetCurrentTarget()->push_back(md_[index]); },
      node.Start() + 1, node.End() - 1);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleStrikeThrough(
    const ParseTreeStrikeThroughNode& node) {
  GetCurrentTarget()->append("<span class='font-strike'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { GetCurrentTarget()->push_back(md_[index]); },
      node.Start() + 2, node.End() - 2);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleLink(const ParseTreeLinkNode& node) {
  assert(("Number of children is not two", node.GetChildren().size() == 2));

  links_.push_back(HTMLLinkBuilder());

  targets_.push_back(&links_.back().link_desc);
  HandleParseTreeNode(*node.GetChildren()[0]);
  targets_.pop_back();

  targets_.push_back(&links_.back().link_url);
  HandleParseTreeNode(*node.GetChildren()[1]);
  targets_.pop_back();

  const HTMLLinkBuilder& link = links_.back();
  GetCurrentTarget()->append(
      StrCat("<a href='", link.link_url, "'>", link.link_desc, "</a>"));
  links_.pop_back();
}

void HTMLGenerator::HandleImage(const ParseTreeImageNode& node) {
  assert(("(Image) Number of children is not two",
          node.GetChildren().size() == 2));

  const ParseTreeNode* desc_node = node.GetChildren()[0].get();
  assert(desc_node->GetNodeType() == ParseTreeNode::NODE);

  const ParseTreeNode* desc = desc_node->GetChildren()[0].get();
  assert(desc->GetNodeType() == ParseTreeNode::TEXT);

  images_.push_back(HTMLImageBuilder());

  const auto keyword_to_index = node.GetKeywordToIndex();
  if (auto index = keyword_to_index.find("alt");
      index != keyword_to_index.end()) {
    targets_.push_back(&images_.back().alt);
    HandleParseTreeNode(*desc->GetChildren().at(index->second));
    targets_.pop_back();
  }

  if (auto index = keyword_to_index.find("caption");
      index != keyword_to_index.end()) {
    targets_.push_back(&images_.back().caption);
    HandleParseTreeNode(*desc->GetChildren().at(index->second));
    targets_.pop_back();
  }

  if (auto index = keyword_to_index.find("size");
      index != keyword_to_index.end()) {
    targets_.push_back(&images_.back().size);
    HandleParseTreeNode(*desc->GetChildren().at(index->second));
    targets_.pop_back();
  }

  targets_.push_back(&images_.back().url);
  HandleParseTreeNode(*node.GetChildren()[1]);
  targets_.pop_back();

  const HTMLImageBuilder& image = images_.back();
  GetCurrentTarget()->append(
      StrCat("<figure><picture><img class='content-img' src='", image.url,
             "' alt='", image.alt, "'>", "</picture><figcaption>",
             image.caption, "</figcaption></figure>"));
  images_.pop_back();
}

void HTMLGenerator::HandleTable(const ParseTreeTableNode& node) {
  GetCurrentTarget()->append("<table><thead><tr>");

  const int row_size = node.GetRowSize();

  // The first row is always the header.
  for (int i = 0; i < row_size; i++) {
    GetCurrentTarget()->append("<th>");
    HandleParseTreeNode(*node.GetChildren()[i]);
    GetCurrentTarget()->append("</th>");
  }

  GetCurrentTarget()->append("</tr></thead><tbody>");

  // The second row of the table always indicates the alignment of the cells.
  // TODO Set the alignmnent of the columns.

  for (int i = 2 * row_size; i < node.GetChildren().size(); i++) {
    if (i % row_size == 0) {
      GetCurrentTarget()->append("<tr>");
    }

    GetCurrentTarget()->append("<td>");
    HandleParseTreeNode(*node.GetChildren()[i]);
    GetCurrentTarget()->append("</td>");

    if (i % row_size == 0) {
      GetCurrentTarget()->append("</tr>");
    }
  }

  GetCurrentTarget()->append("</tbody></table>");
}

void HTMLGenerator::HandleList(const ParseTreeListNode& node) {
  if (node.IsOrdered()) {
    GetCurrentTarget()->append("<ol>");
  } else {
    GetCurrentTarget()->append("<ul>");
  }

  for (const auto& child : node.GetChildren()) {
    HandleParseTreeNode(*child);
  }

  if (node.IsOrdered()) {
    GetCurrentTarget()->append("</ol>");
  } else {
    GetCurrentTarget()->append("</ul>");
  }
}

void HTMLGenerator::HandleListItem(const ParseTreeListItemNode& node) {
  GetCurrentTarget()->append("<li>");

  for (const auto& child : node.GetChildren()) {
    HandleParseTreeNode(*child);
  }

  GetCurrentTarget()->append("</li>");
}

void HTMLGenerator::HandleHeader(const ParseTreeHeaderNode& node) {
  assert(node.GetChildren().size() == 2);

  if (node.GetHeaderType() == ParseTreeHeaderNode::NORMAL_HEADER) {
    const auto& header_symbol = node.GetChildren()[0];
    GetCurrentTarget()->append(StrCat(
        "<h", std::to_string(header_symbol->End() - header_symbol->Start()),
        " class='header-general'>"));
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append(StrCat(
        "</h", std::to_string(header_symbol->End() - header_symbol->Start()),
        ">"));
  }
}

// If the verbatim node does not have any child node, then emit the current text
// as verbatim. If it does have children, then the first node tells the type of
// code it is showing and the second node contains the actual code.
void HTMLGenerator::HandleVerbatim(const ParseTreeVerbatimNode& node) {
  if (node.GetChildren().empty()) {
    // Then this is the inline.
    GetCurrentTarget()->append("<code class='inline-code'>");

    // [start + 1 ~ end - 1)
    GetCurrentTarget()->append(
        md_.substr(node.Start() + 1, node.End() - node.Start() - 2));
    GetCurrentTarget()->append("</code>");
  }
}

void HTMLGenerator::HandleEscape(const ParseTreeEscapeNode& node) {
  GetCurrentTarget()->push_back(md_[node.Start() + 1]);
}

void HTMLGenerator::HandleCommand(const ParseTreeCommandNode& node) {
  std::string_view command = node.GetCommandName();
  if (command == "sidenote") {
    GetCurrentTarget()->append("<aside class='sidenote'>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</aside>");
  } else if (command == "sc") {
    GetCurrentTarget()->append("<span class='font-smallcaps'>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</span>");
  } else if (command == "serif") {
    GetCurrentTarget()->append("<span class='font-serif-italic'>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</span>");
  } else if (command == "htmlonly") {
    HandleParseTreeNode(*node.GetChildren()[0]);
  } else if (command == "escape") {
    HandleParseTreeNode(*node.GetChildren()[0]);
  } else if (command == "footnote") {
    GetCurrentTarget()->append("<sup>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</sup>");
  } else if (command == "tooltip") {
    assert(node.GetChildren().size() == 2);

    GetCurrentTarget()->append("<span class='page-tooltip' data-tooltip='");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("' data-tooltip-position='bottom'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</span>");
  }
}

}  // namespace md2


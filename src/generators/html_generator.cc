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
    case ParseTreeNode::MATH:
      HandleMath(CastNodeTypes<ParseTreeMathNode>(node));
      break;
    case ParseTreeNode::BOX:
      HandleBox(CastNodeTypes<ParseTreeBoxNode>(node));
      break;
    default:
      break;
  }
}

void HTMLGenerator::EmitChar(int index) {
  if (should_escape_html_) {
    if (md_[index] == '<') {
      GetCurrentTarget()->append("&lt;");
    } else if (md_[index] == '>') {
      GetCurrentTarget()->append("&gt;");
    } else if (md_[index] == '&') {
      GetCurrentTarget()->append("&amp;");
    } else {
      GetCurrentTarget()->push_back(md_[index]);
    }
  } else {
    GetCurrentTarget()->push_back(md_[index]);
  }
}

void HTMLGenerator::EmitChar(int from, int to) {
  for (int i = from; i < to; i++) {
    EmitChar(i);
  }
}

void HTMLGenerator::HandleParagraph(const ParseTreeParagraphNode& node) {
  GetCurrentTarget()->append("<p>");

  GenerateWithDefaultAction(node, [this](int index) { EmitChar(index); });

  GetCurrentTarget()->append("</p>");
}

void HTMLGenerator::HandleText(const ParseTreeTextNode& node) {
  GenerateWithDefaultAction(node, [this](int index) { EmitChar(index); });
}

void HTMLGenerator::HandleBold(const ParseTreeBoldNode& node) {
  GetCurrentTarget()->append("<span class='font-weight-bold'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 2,
      node.End() - 2);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleItalic(const ParseTreeItalicNode& node) {
  GetCurrentTarget()->append("<span class='font-italic'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 1,
      node.End() - 1);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleStrikeThrough(
    const ParseTreeStrikeThroughNode& node) {
  GetCurrentTarget()->append("<span class='font-strike'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 2,
      node.End() - 2);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleLink(const ParseTreeLinkNode& node) {
  ASSERT(node.GetChildren().size() == 2, "Number of children is not two");

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
  ASSERT(node.GetChildren().size() == 2, "Number of children is not two");

  const ParseTreeNode* desc_node = node.GetChildren()[0].get();
  ASSERT(desc_node->GetNodeType() == ParseTreeNode::NODE, "");

  const ParseTreeNode* desc = desc_node->GetChildren()[0].get();
  ASSERT(desc->GetNodeType() == ParseTreeNode::TEXT, "");

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

  for (size_t i = 2 * row_size; i < node.GetChildren().size(); i++) {
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
  ASSERT(node.GetChildren().size() == 2, "");

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
    EmitChar(node.Start() + 1, node.End() - 1);
    GetCurrentTarget()->append("</code>");
    return;
  }

  ASSERT(node.GetChildren().size() == 2, "Verbatim does not have two nodes.");
  const auto& code_name_node = node.GetChildren()[0];
  std::string_view code_name = md_.substr(
      code_name_node->Start(), code_name_node->End() - code_name_node->Start());

  const auto& code_node = node.GetChildren()[1];
  std::string_view code =
      md_.substr(code_node->Start(), code_node->End() - code_node->Start());
  if (code_name == "cpp") {
    GetCurrentTarget()->append("<pre class='chroma lang-cpp plain-code'>");
    GetCurrentTarget()->append(code);
    GetCurrentTarget()->append("</pre>");
  } else if (code_name == "embed") {
    GetCurrentTarget()->append(code);
  }
}

void HTMLGenerator::HandleEscape(const ParseTreeEscapeNode& node) {
  EmitChar(node.Start() + 1);
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
    ASSERT(node.GetChildren().size() == 2, "");

    GetCurrentTarget()->append("<span class='page-tooltip' data-tooltip='");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("' data-tooltip-position='bottom'>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</span>");
  }
}

void HTMLGenerator::HandleMath(const ParseTreeMathNode& node) {
  GetCurrentTarget()->append("<span class='math-latex'>");
  // Math should be enclosed by $$.
  EmitChar(node.Start() + 1, node.End() - 1);
  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleBox(const ParseTreeBoxNode& node) {
  ASSERT(node.GetChildren().size() == 2, "");
  const auto& box_name_node = node.GetChildren()[0];

  std::string_view box_name = md_.substr(
      box_name_node->Start(), box_name_node->End() - box_name_node->Start());

  if (box_name == "info-text") {
    GetCurrentTarget()->append("<div class='info'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name == "exec") {
    GetCurrentTarget()->append(
        "<p class='exec-preview-title'>실행 결과</p><pre "
        "class='exec-preview'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</pre>");
  } else if (box_name == "warning") {
    GetCurrentTarget()->append("<div class='warning warning-text'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name == "lec-warning") {
    GetCurrentTarget()->append(
        "<p class='compiler-warning-title'><i class='xi-warning'></i>주의 "
        "사항</p><div class='lec-warning'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name == "lec-info") {
    GetCurrentTarget()->append(
        "<p class='lec-info-title'><i class='xi-info'></i>참고 사항</p><div "
        "class='lec-info'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name == "lec-summary") {
    GetCurrentTarget()->append(
        "<div class='lec-summary'><h3>뭘 배웠지?</h3><div "
        "class='lec-summary-content'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div></div>");
  } else if (box_name == "html-only") {
    HandleParseTreeNode(*node.GetChildren()[1]);
  } else if (box_name == "latex-only") {
    return;
  } else if (box_name == "sidenote") {
    GetCurrentTarget()->append("<aside class='sidenote'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</aside>");
  } else if (box_name == "note") {
    GetCurrentTarget()->append("<div class='inline-note'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  }
}

}  // namespace md2


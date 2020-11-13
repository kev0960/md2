#include "latex_generator.h"

#include "generator_util.h"

namespace md2 {
namespace {

std::string_view EscapeLatexChar(char c) {
  switch (c) {
    case '~':
      return "$\\sim$";
    case '^':
      return "$\\hat$";
    case '\\':
      return "\\textbackslash";
    case '&':
      return "\\&";
    case '%':
      return "\\%";
    case '$':
      return "\\$";
    case '#':
      return "\\#";
    case '_':
      return "\\_";
    case '{':
      return "\\{";
    case '}':
      return "\\}";
  }

  return "";
}

}  // namespace

void LatexGenerator::HandleParseTreeNode(const ParseTreeNode& node) {
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
    case ParseTreeNode::STRIKE_THROUGH:
      HandleStrikeThrough(CastNodeTypes<ParseTreeStrikeThroughNode>(node));
      break;
    /*
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
    case ParseTreeNode::MATH:
      HandleMath(CastNodeTypes<ParseTreeMathNode>(node));
      break;
    case ParseTreeNode::BOX:
      HandleBox(CastNodeTypes<ParseTreeBoxNode>(node));
      break;
    case ParseTreeNode::QUOTE:
      HandleQuote(CastNodeTypes<ParseTreeQuoteNode>(node));
      break;
    */
    default:
      break;
  }
}

void LatexGenerator::EmitChar(int index) {
  if (should_escape_latex_) {
    std::string_view escaped = EscapeLatexChar(md_[index]);
    if (!escaped.empty()) {
      GetCurrentTarget()->append(escaped);
    } else
      GetCurrentTarget()->push_back(md_[index]);
  } else {
    GetCurrentTarget()->push_back(md_[index]);
  }
}

void LatexGenerator::EmitChar(int from, int to) {
  for (int i = from; i < to; i++) {
    EmitChar(i);
  }
}

void LatexGenerator::HandleParagraph(const ParseTreeParagraphNode& node) {
  GetCurrentTarget()->append("\n");
  GenerateWithDefaultAction(node, [this](int index) { EmitChar(index); });
  GetCurrentTarget()->append("\n");
}

void LatexGenerator::HandleText(const ParseTreeTextNode& node) {
  GenerateWithDefaultAction(node, [this](int index) { EmitChar(index); });
}

void LatexGenerator::HandleBold(const ParseTreeBoldNode& node) {
  GetCurrentTarget()->append("\\textbf{");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 2,
      node.End() - 2);

  GetCurrentTarget()->append("}");
}

void LatexGenerator::HandleItalic(const ParseTreeItalicNode& node) {
  GetCurrentTarget()->append("\\emph{");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 1,
      node.End() - 1);

  GetCurrentTarget()->append("}");
}

void LatexGenerator::HandleStrikeThrough(
    const ParseTreeStrikeThroughNode& node) {
  GetCurrentTarget()->append("\\sout{");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 2,
      node.End() - 2);

  GetCurrentTarget()->append("}");
}

}  // namespace md2

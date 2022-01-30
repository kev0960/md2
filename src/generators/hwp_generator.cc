#include "hwp_generator.h"

#include "fmt/format.h"
#include "generator_util.h"

namespace md2 {
namespace {

constexpr std::string_view kMathEquationTag =
    R"(<EQUATION BaseLine="66" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60">)";

constexpr std::string_view kMathShapeObject =
    R"(<SHAPEOBJECT InstId="{}" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="{}">)";

constexpr std::string_view kMathPositionAndMargin =
    R"(<POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT>)";

constexpr std::string_view kMathHeightAndWidth =
    R"(<SIZE Height="{}" HeightRelTo="Absolute" Protect="false" Width="{}" WidthRelTo="Absolute"/>)";

// Height and Width is embedded in math as follows:
// $$150,200;a+b+c$$
//
// Returns the pair of height and width, and the start index of the real math.
std::pair<std::pair<int, int>, int> GetHeightAndWidthOfMath(
    std::string_view math) {
  size_t height_start = 2;
  size_t height_end = math.find(',', height_start);

  int height = std::stoi(
      std::string(math.substr(height_start, height_end - height_start)));

  size_t width_end = math.find(';', height_end + 1);
  int width = std::stoi(
      std::string(math.substr(height_end + 1, width_end - (height_end + 1))));

  return std::make_pair(std::make_pair(height, width), width_end + 1);
}

}  // namespace

void HwpGenerator::EmitChar(int index) {
  switch (md_[index]) {
    case '\t': {
      GetCurrentTarget()->append("<TAB/>");
      break;
    }
    case '"': {
      GetCurrentTarget()->append("&quot;");
      break;
    }
    case '\'': {
      GetCurrentTarget()->append("&apos;");
      break;
    }
    case '<': {
      GetCurrentTarget()->append("&lt;");
      break;
    }
    case '>': {
      GetCurrentTarget()->append("&gt;");
      break;
    }
    case '&': {
      GetCurrentTarget()->append("&amp;");
      break;
    }
    default: {
      GetCurrentTarget()->push_back(md_[index]);
      break;
    }
  }
}

void HwpGenerator::EmitChar(int from, int to) {
  for (int i = from; i < to; i++) {
    EmitChar(i);
  }
}

void HwpGenerator::HandleParseTreeNode(const ParseTreeNode& node) {
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
    case ParseTreeNode::MATH_NEWLINE:
      HandleNewlineMath(CastNodeTypes<ParseTreeNewlineMathNode>(node));
      break;
    case ParseTreeNode::BOX:
      HandleBox(CastNodeTypes<ParseTreeBoxNode>(node));
      break;
    case ParseTreeNode::QUOTE:
      HandleQuote(CastNodeTypes<ParseTreeQuoteNode>(node));
      break;
    default:
      break;
  }
}

void HwpGenerator::HandleParagraph(const ParseTreeParagraphNode& node) {
  // Do not emit the empty or mal-formed paragraph.
  if (node.Start() >= node.End()) {
    return;
  }

  auto [shape, style] =
      hwp_state_manager_.GetParaShape(HwpStateManager::PARA_REGULAR);
  GetCurrentTarget()->append(
      fmt::format(R"(<P ParaShape="{}" Style="{}">)", shape, style));

  paragraph_nest_count_++;

  int current_start = node.Start();
  for (const auto& child_node : node.GetChildren()) {
    // Handle the regular texts.
    if (current_start < child_node->Start()) {
      GetCurrentTarget()->append(fmt::format(
          R"(<TEXT CharShape="{}"><CHAR>)",
          hwp_state_manager_.GetCharShape(HwpStateManager::CHAR_REGULAR)));
      EmitChar(current_start, child_node->Start());
      GetCurrentTarget()->append("</CHAR></TEXT>");
    }

    HandleParseTreeNode(*child_node);
    current_start = child_node->End();
  }

  if (current_start < node.End()) {
    GetCurrentTarget()->append(fmt::format(
        R"(<TEXT CharShape="{}"><CHAR>)",
        hwp_state_manager_.GetCharShape(HwpStateManager::CHAR_REGULAR)));
    EmitChar(current_start, node.End());
    GetCurrentTarget()->append("</CHAR></TEXT>");
  }

  GetCurrentTarget()->append("</P>");

  paragraph_nest_count_--;
}

void HwpGenerator::HandleMath(const ParseTreeMathNode& node) {
  ParagraphWrapper wrapper(this, /*wrap_text=*/true);

  GetCurrentTarget()->append("<TEXT>");
  GetCurrentTarget()->append(kMathEquationTag);
  GetCurrentTarget()->append(
      fmt::format(kMathShapeObject, inst_id_++, z_order_++));

  std::string_view math_in_node = GetStringInNode(&node);

  auto [height_and_width, actual_start_offset] =
      GetHeightAndWidthOfMath(math_in_node);

  GetCurrentTarget()->append(fmt::format(
      kMathHeightAndWidth, height_and_width.first, height_and_width.second));
  GetCurrentTarget()->append(kMathPositionAndMargin);
  GetCurrentTarget()->append("</SHAPEOBJECT><SCRIPT>");

  // We have to omit ending "$$".
  EmitChar(node.Start() + actual_start_offset, node.End() - 2);

  GetCurrentTarget()->append("</SCRIPT></EQUATION>");
  GetCurrentTarget()->append("</TEXT>");
}

void HwpGenerator::HandleNewlineMath(const ParseTreeNewlineMathNode& node) {
  ParagraphWrapper wrapper(this, /*wrap_text=*/true);

  GetCurrentTarget()->append("<TEXT>");
  GetCurrentTarget()->append(kMathEquationTag);
  GetCurrentTarget()->append(
      fmt::format(kMathShapeObject, inst_id_++, z_order_++));

  std::string_view math_in_node = GetStringInNode(&node);
  auto [height_and_width, actual_start_offset] =
      GetHeightAndWidthOfMath(math_in_node);

  GetCurrentTarget()->append(fmt::format(
      kMathHeightAndWidth, height_and_width.first, height_and_width.second));
  GetCurrentTarget()->append(kMathPositionAndMargin);
  GetCurrentTarget()->append("</SHAPEOBJECT><SCRIPT>");

  // We have to omit ending "\]".
  EmitChar(node.Start() + actual_start_offset, node.End() - 2);

  GetCurrentTarget()->append("</SCRIPT></EQUATION>");
  GetCurrentTarget()->append("</TEXT>");
}

void HwpGenerator::HandleText(const ParseTreeTextNode& node) {
  // Do not handle text node.
  (void)node;
}

void HwpGenerator::HandleBold(const ParseTreeBoldNode& node) {
  ParagraphWrapper wrapper(this);

  GetCurrentTarget()->append(
      fmt::format(R"(<TEXT CharShape="{}"><CHAR>)",
                  hwp_state_manager_.GetCharShape(HwpStateManager::BOLD)));
  EmitChar(node.Start(), node.End());
  GetCurrentTarget()->append("</CHAR></TEXT>");
}

void HwpGenerator::HandleItalic(const ParseTreeItalicNode& node) {
  ParagraphWrapper wrapper(this);

  GetCurrentTarget()->append(
      fmt::format(R"(<TEXT CharShape="{}"><CHAR>)",
                  hwp_state_manager_.GetCharShape(HwpStateManager::ITALIC)));
  EmitChar(node.Start(), node.End());
  GetCurrentTarget()->append("</CHAR></TEXT>");
}

void HwpGenerator::HandleStrikeThrough(const ParseTreeStrikeThroughNode& node) {
  // Do not handle strike through for Hwp.
  (void)node;
}
void HwpGenerator::HandleLink(const ParseTreeLinkNode& node) {
  // Do not handle links for Hwp.
  (void)node;
}
void HwpGenerator::HandleImage(const ParseTreeImageNode& node) { (void)node; }
void HwpGenerator::HandleTable(const ParseTreeTableNode& node) { (void)node; }
void HwpGenerator::HandleList(const ParseTreeListNode& node) { (void)node; }
void HwpGenerator::HandleListItem(const ParseTreeListItemNode& node) {
  (void)node;
}
void HwpGenerator::HandleHeader(const ParseTreeHeaderNode& node) { (void)node; }
void HwpGenerator::HandleVerbatim(const ParseTreeVerbatimNode& node) {
  (void)node;
}
void HwpGenerator::HandleEscape(const ParseTreeEscapeNode& node) { (void)node; }
void HwpGenerator::HandleCommand(const ParseTreeCommandNode& node) {
  (void)node;
}

void HwpGenerator::HandleBox(const ParseTreeBoxNode& node) { (void)node; }
void HwpGenerator::HandleQuote(const ParseTreeQuoteNode& node) { (void)node; }

}  // namespace md2


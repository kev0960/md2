#include "hwp_generator.h"

#include "fmt/format.h"
#include "generator_util.h"
#include "string_util.h"

namespace md2 {
namespace {

constexpr std::string_view kMathEquationTag =
    R"(<EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60">)";

constexpr std::string_view kMathShapeObject =
    R"(<SHAPEOBJECT InstId="{}" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="{}">)";

constexpr std::string_view kMathPositionAndMargin =
    R"(<POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT>)";

constexpr std::string_view kMathHeightAndWidth =
    R"(<SIZE Height="{}" HeightRelTo="Absolute" Protect="false" Width="{}" WidthRelTo="Absolute"/>)";

constexpr std::string_view kTableHeader =
    R"(<TABLE BorderFill="11" CellSpacing="0" ColCount="{}" PageBreak="Cell" RepeatHeader="true" RowCount="{}">)";

constexpr std::string_view kTableShapeObject =
    R"(<SHAPEOBJECT InstId="{}" Lock="false" NumberingType="Table" TextFlow="LargestOnly" ZOrder="{}"><SIZE Height="{}" HeightRelTo="Absolute" Protect="false" Width="{}" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="850" Right="0" Top="0"/></SHAPEOBJECT><INSIDEMARGIN Bottom="141" Left="510" Right="510" Top="141"/>)";

constexpr std::string_view kCell =
    R"(<CELL BorderFill="11" ColAddr="{}" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="{}" Protect="false" RowAddr="{}" RowSpan="1" Width="{}"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center">)";

// Inst Id, ZOrder, Height, Width
// CurHeight CurWidth
// Inst Id (Different from above)
// OriHeight OriWidth XPos YPos
// CenterX (= Width / 2) CenterY (= Height / 2)
// TransMatrix E3, E6
// ScaleMatrix E1, E3, E5, E6
// X1, X2 = Width
// Y2, Y3 = Height
// Bottom = Height Left Right = Width Top
// BinItem
constexpr std::string_view kImage =
    R"(<PICTURE Reverse="false"><SHAPEOBJECT InstId="{}" Lock="false" NumberingType="Figure" ZOrder="{}"><SIZE Height="{}" HeightRelTo="Absolute" Protect="false" Width="{}" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="0" Right="0" Top="0"/><SHAPECOMMENT></SHAPECOMMENT></SHAPEOBJECT><SHAPECOMPONENT CurHeight="{}" CurWidth="{}" GroupLevel="0" HorzFlip="false" InstID="{}" OriHeight="{}" OriWidth="{}" VertFlip="false" XPos="{}" YPos="{}"><ROTATIONINFO Angle="0" CenterX="{}" CenterY="{}" Rotate="1"/><RENDERINGINFO><TRANSMATRIX E1="1.00000" E2="0.00000" E3="{:.5f}" E4="0.00000" E5="1.00000" E6="{:.5f}"/><SCAMATRIX E1="{:.5f}" E2="0.00000" E3="{:.5f}" E4="0.00000" E5="{:.5f}" E6="{:.5f}"/><ROTMATRIX E1="1.00000" E2="0.00000" E3="0.00000" E4="0.00000" E5="1.00000" E6="0.00000"/></RENDERINGINFO></SHAPECOMPONENT><IMAGERECT X0="0" X1="{}" X2="{}" X3="0" Y0="0" Y1="0" Y2="{}" Y3="{}"/><IMAGECLIP Bottom="{}" Left="{}" Right="{}" Top="{}"/><INSIDEMARGIN Bottom="0" Left="0" Right="0" Top="0"/><IMAGE Alpha="0" BinItem="{}" Bright="0" Contrast="0" Effect="RealPic"/><EFFECTS/></PICTURE>)";

constexpr int kTableFullWidth = 42000;

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

// Height and Width is embedded in the image path as follows:
// ![](100,120)
//
// It's because Hwp generator doesn't read the image file. It only uses height
// and width info.
std::pair<int, int> GetHeightAndWidthOfImage(std::string_view image_path) {
  size_t height_end = image_path.find(',');

  int height = std::stoi(std::string(image_path.substr(0, height_end)));
  int width = std::stoi(std::string(image_path.substr(height_end + 1)));

  return std::make_pair(height, width);
}

std::string GetPrefixForListItem(std::string_view box_name, int index) {
  static std::vector<std::string> candidate_prefix = {
      "① ", "<TAB />② ", "<TAB />③ ", "<TAB />④ ", "<TAB />⑤ "};
  static std::vector<std::string> example_prefix = {"ㄱ. ", "ㄴ. ", "ㄷ. ",
                                                    "ㄹ. ", "ㅁ. "};
  static std::vector<std::string> condition_prefix = {"(가) ", "(나) ", "(다) ",
                                                      "(라) ", "(마) "};

  if (index >= 5) {
    return std::to_string(index + 1);
  }

  if (box_name == "candidates") {
    return candidate_prefix[index];
  } else if (box_name == "examples") {
    return example_prefix[index];
  } else if (box_name == "conditions") {
    return condition_prefix[index];
  }

  return std::to_string(index + 1);
}

struct HwpImageSize {
  int cur_height;
  int cur_width;
  int ori_height;
  int ori_width;
  int xpos, ypos;
  int bottom, left, right, top;
};

std::optional<HwpImageSize> GetHwpImageSize(std::string_view image_size) {
  if (image_size.empty()) {
    return std::nullopt;
  }

  std::vector<std::string_view> sizes = SplitStringByChar(image_size, ',');
  if (sizes.size() == 0) {
    return std::nullopt;
  }

  HwpImageSize size;

  if (sizes.size() == 4) {
    size.cur_height = std::stoi(std::string(sizes[0]));
    size.cur_width = std::stoi(std::string(sizes[1]));
    size.ori_height = std::stoi(std::string(sizes[2]));
    size.ori_width = std::stoi(std::string(sizes[3]));
    size.xpos = 0;
    size.ypos = 0;
    size.bottom = size.ori_height;
    size.left = 0;
    size.right = size.ori_width;
    size.top = 0;
  } else if (sizes.size() == 10) {
    size.cur_height = std::stoi(std::string(sizes[0]));
    size.cur_width = std::stoi(std::string(sizes[1]));
    size.ori_height = std::stoi(std::string(sizes[2]));
    size.ori_width = std::stoi(std::string(sizes[3]));
    size.xpos = std::stoi(std::string(sizes[4]));
    size.ypos = std::stoi(std::string(sizes[5]));
    size.bottom = std::stoi(std::string(sizes[6]));
    size.left = std::stoi(std::string(sizes[7]));
    size.right = std::stoi(std::string(sizes[8]));
    size.top = std::stoi(std::string(sizes[9]));
  } else {
    return std::nullopt;
  }

  return size;
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

  bool p_added = false;
  if (!NestedInP()) {
    auto [shape, style] =
        hwp_state_manager_.GetRegularParaShape(total_paragraph_count_);
    GetCurrentTarget()->append(
        fmt::format(R"(<P ParaShape="{}" Style="{}">)", shape, style));

    xml_tree_.push_back(HwpXmlTag::P);
    p_added = true;
  }

  int current_start = node.Start();
  for (const auto& child_node : node.GetChildren()) {
    // Handle the regular texts.
    if (current_start < child_node->Start()) {
      TextWrapper text_wrapper(
          this, hwp_state_manager_.GetRegularCharShape(total_paragraph_count_));

      GetCurrentTarget()->append("<CHAR>");
      EmitChar(current_start, child_node->Start());
      GetCurrentTarget()->append("</CHAR>");
    }

    HandleParseTreeNode(*child_node);
    current_start = child_node->End();
  }

  if (current_start < node.End()) {
    TextWrapper text_wrapper(
        this, hwp_state_manager_.GetRegularCharShape(total_paragraph_count_));

    GetCurrentTarget()->append("<CHAR>");
    EmitChar(current_start, node.End());
    GetCurrentTarget()->append("</CHAR>");
  }

  if (p_added) {
    xml_tree_.pop_back();
    GetCurrentTarget()->append("</P>");

    // Only increase paragraph count if the paragraph is actuallly added.
    total_paragraph_count_++;
  }
}

void HwpGenerator::HandleMath(const ParseTreeMathNode& node) {
  ParagraphWrapper wrapper(this);
  TextWrapper text_wrapper(this, 0);

  GetCurrentTarget()->append(kMathEquationTag);
  GetCurrentTarget()->append(fmt::format(
      kMathShapeObject, hwp_status_.inst_id++, hwp_status_.z_order++));

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
}

void HwpGenerator::HandleNewlineMath(const ParseTreeNewlineMathNode& node) {
  ParagraphWrapper wrapper(this);
  TextWrapper text_wrapper(this, 0);

  GetCurrentTarget()->append(kMathEquationTag);
  GetCurrentTarget()->append(fmt::format(
      kMathShapeObject, hwp_status_.inst_id++, hwp_status_.z_order++));

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
}

void HwpGenerator::HandleText(const ParseTreeTextNode& node) {
  if (targets_.size() <= 1) {
    return;
  }

  if (node.Start() >= node.End()) {
    return;
  }

  // We only handle text for the nested case (e.g description in the image.)
  EmitChar(node.Start(), node.End());
}

void HwpGenerator::HandleBold(const ParseTreeBoldNode& node) {
  ParagraphWrapper wrapper(this);

  TextWrapper text_wrapper(
      this, hwp_state_manager_.GetCharShape(HwpStateManager::BOLD));

  GetCurrentTarget()->append("<CHAR>");
  EmitChar(node.Start(), node.End());
  GetCurrentTarget()->append("</CHAR>");
}

void HwpGenerator::HandleItalic(const ParseTreeItalicNode& node) {
  ParagraphWrapper wrapper(this);

  TextWrapper text_wrapper(
      this, hwp_state_manager_.GetCharShape(HwpStateManager::ITALIC));

  GetCurrentTarget()->append("<CHAR>");
  EmitChar(node.Start(), node.End());
  GetCurrentTarget()->append("</CHAR>");
}

void HwpGenerator::HandleStrikeThrough(const ParseTreeStrikeThroughNode& node) {
  // Do not handle strike through for Hwp.
  (void)node;
}
void HwpGenerator::HandleLink(const ParseTreeLinkNode& node) {
  // Do not handle links for Hwp.
  (void)node;
}
void HwpGenerator::HandleImage(const ParseTreeImageNode& node) {
  ParagraphWrapper wrapper(this);
  TextWrapper text_wrapper(this, 0);

  const ParseTreeNode* desc_node = node.GetChildren()[0].get();
  MD2_ASSERT(desc_node->GetNodeType() == ParseTreeNode::NODE, "");

  const ParseTreeNode* desc = desc_node->GetChildren()[0].get();
  MD2_ASSERT(desc->GetNodeType() == ParseTreeNode::TEXT, "");

  const auto keyword_to_index = node.GetKeywordToIndex();
  std::string image_size;
  if (auto index = keyword_to_index.find("size");
      index != keyword_to_index.end()) {
    targets_.push_back(&image_size);
    HandleParseTreeNode(*desc->GetChildren().at(index->second));
    targets_.pop_back();
  }

  std::optional<HwpImageSize> size = GetHwpImageSize(image_size);
  if (!size.has_value()) {
    int image_text_start = node.GetChildren()[1]->Start();
    int image_text_end = node.GetChildren()[1]->End();

    auto [height, width] = GetHeightAndWidthOfImage(md_.substr(
        image_text_start + 1, image_text_end - image_text_start - 2));

    // Inst Id, ZOrder, CurHeight, CurWidth
    // CurHeight CurWidth
    // Inst Id (Different from above)
    // OriHeight OriWidth XPos YPos
    // CenterX (= CurWidth / 2) CenterY (= CurHeight / 2)
    // TransMatrix E3, E6
    // ScaleMatrix E1, E3, E5, E6
    // X1, X2 = Width
    // Y2, Y3 = Height
    // Bottom = Height Left Right = Width Top
    // BinItem

    GetCurrentTarget()->append(fmt::format(
        kImage, hwp_status_.inst_id, hwp_status_.z_order++, height, width,
        height, width, hwp_status_.inst_id + 1, height, width, /*xpos=*/0,
        /*ypos=*/0, width / 2, height / 2, /*E3=*/0.f, /*E6=*/0.f, 1.f, 0.f,
        1.f, 0.f, width, width, height, height, /*bottom=*/height, /*left=*/0,
        /*right=*/width,
        /*top=*/0, hwp_status_.bin_item++));
  } else {
    const HwpImageSize& sz = *size;
    GetCurrentTarget()->append(fmt::format(
        kImage, hwp_status_.inst_id, hwp_status_.z_order++, sz.cur_height,
        sz.cur_width, sz.cur_height, sz.cur_width, hwp_status_.inst_id + 1,
        sz.ori_height, sz.ori_width, /*xpos=*/sz.xpos, /*ypos=*/sz.ypos,
        sz.cur_width / 2, sz.cur_height / 2, static_cast<float>(sz.xpos),
        static_cast<float>(sz.ypos),
        /*E1=*/static_cast<float>(sz.cur_width) / sz.ori_width,
        /*E3=*/sz.xpos == 0 ? 0.f : -static_cast<float>(sz.xpos),
        /*E5=*/static_cast<float>(sz.cur_height) / sz.ori_height,
        /*E6=*/sz.ypos == 0 ? 0.f : -static_cast<float>(sz.ypos), sz.ori_width,
        sz.ori_width, sz.ori_height, sz.ori_height, sz.bottom, sz.left,
        sz.right, sz.top, hwp_status_.bin_item++));
  }

  hwp_status_.inst_id += 2;
}

void HwpGenerator::HandleTable(const ParseTreeTableNode& node) {
  ParagraphWrapper wrapper(this);
  TextWrapper text_wrapper(
      this, hwp_state_manager_.GetCharShape(HwpStateManager::CHAR_REGULAR));

  const size_t col_size = node.GetColSize();
  const size_t row_size = node.GetChildren().size() / col_size - 1;

  const int height = 1700 * row_size;

  GetCurrentTarget()->append(fmt::format(kTableHeader, col_size, row_size));
  GetCurrentTarget()->append(
      fmt::format(kTableShapeObject, hwp_status_.inst_id++,
                  hwp_status_.z_order++, height, kTableFullWidth));

  int row_index = 0;
  for (size_t i = 0; i < node.GetChildren().size(); i++) {
    if (col_size <= i && i < col_size * 2) {
      // Ignore the second row.
      continue;
    }

    if (i % col_size == 0) {
      // This is the start of the row.
      GetCurrentTarget()->append("<ROW>");
      xml_tree_.push_back(HwpXmlTag::ROW);
    }

    GetCurrentTarget()->append(fmt::format(
        kCell, /*col_addr=*/i % col_size, /*height=*/1700,
        /*row_addr=*/row_index, /*width=*/kTableFullWidth / col_size));
    xml_tree_.push_back(HwpXmlTag::CELL);
    xml_tree_.push_back(HwpXmlTag::PARALIST);

    HandleParseTreeNode(*node.GetChildren()[i]);

    GetCurrentTarget()->append("</PARALIST></CELL>");
    xml_tree_.pop_back();
    xml_tree_.pop_back();

    if (i % col_size == col_size - 1) {
      // This is the end of the row.
      GetCurrentTarget()->append("</ROW>");
      xml_tree_.pop_back();
      row_index++;
    }
  }

  GetCurrentTarget()->append("</TABLE>");
}

void HwpGenerator::HandleList(const ParseTreeListNode& node) {
  // Only wrap with paragraph for candidate box env.
  ParagraphWrapper wrapper(this, /*skip=*/!IsInBoxEnvironment("candidates"));

  for (const auto& child : node.GetChildren()) {
    HandleParseTreeNode(*child);
  }
}

void HwpGenerator::HandleListItem(const ParseTreeListItemNode& node) {
  // Items in "candiadates" should be rendered in a horizontal fashion so we
  // should not render P tag.
  ParagraphWrapper wrapper(this, /*skip=*/IsInBoxEnvironment("candidates"));
  TextWrapper text_wrapper(
      this, hwp_state_manager_.GetCharShape(HwpStateManager::CHAR_REGULAR));

  std::string prefix;
  if (IsInBoxEnvironment("candidates")) {
    prefix = GetPrefixForListItem("candidates", node.ListIndex());
  } else if (IsInBoxEnvironment("examples")) {
    prefix = GetPrefixForListItem("examples", node.ListIndex());
  } else if (IsInBoxEnvironment("conditions")) {
    prefix = GetPrefixForListItem("conditions", node.ListIndex());
  }

  GetCurrentTarget()->append(fmt::format("<CHAR>{}</CHAR>", prefix));

  for (const auto& child : node.GetChildren()) {
    HandleParseTreeNode(*child);
  }
}

void HwpGenerator::HandleHeader(const ParseTreeHeaderNode& node) { (void)node; }
void HwpGenerator::HandleVerbatim(const ParseTreeVerbatimNode& node) {
  (void)node;
}
void HwpGenerator::HandleEscape(const ParseTreeEscapeNode& node) { (void)node; }
void HwpGenerator::HandleCommand(const ParseTreeCommandNode& node) {
  (void)node;
}

void HwpGenerator::HandleBox(const ParseTreeBoxNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "");
  const auto& box_name_node = node.GetChildren()[0];

  std::string_view box_name = md_.substr(
      box_name_node->Start(), box_name_node->End() - box_name_node->Start());

  BoxInserter box_inserter(&current_box_, box_name);

  if (box_name == "conditions") {
    HandleParseTreeNode(*node.GetChildren()[1]);
  } else if (box_name == "examples") {
    HandleParseTreeNode(*node.GetChildren()[1]);
  } else if (box_name == "candidates") {
    HandleParseTreeNode(*node.GetChildren()[1]);
  }
}

void HwpGenerator::HandleQuote(const ParseTreeQuoteNode& node) { (void)node; }

}  // namespace md2


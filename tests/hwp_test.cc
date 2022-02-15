#include "../src/generators/hwp_generator.h"
#include "../src/parser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace md2 {
namespace {

using ::testing::Eq;

void DoHwpTest(std::string content, std::string expected,
               bool is_server_mode = false) {
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  MetadataRepo repo;
  GeneratorOptions options;
  options.server_mode = is_server_mode;

  GeneratorContext context(repo, "image_path", /*use_clang_server=*/false,
                           /*clang_server_port=*/0, nullptr, options);
  HwpGenerator generator(/*filename=*/"some_file.md", content, context, tree);
  generator.Generate();

  std::cout << generator.ShowOutput() << std::endl;
  EXPECT_EQ(std::string(generator.ShowOutput()), expected);
}

TEST(HwpTest, Paragraph) {
  DoHwpTest(
      "a",
      R"(<P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>a</CHAR></TEXT></P>)");
}

TEST(HwpTest, SimpleMath) {
  DoHwpTest(
      "$$125,1000;a+b$$",
      R"(<P ParaShape="0" Style="0"><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="1" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="1"><SIZE Height="125" HeightRelTo="Absolute" Protect="false" Width="1000" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>a+b</SCRIPT></EQUATION></TEXT></P>)");
}

TEST(HwpTest, MathWithText) {
  DoHwpTest(
      R"(there is $$125,10;a+b$$ something \[10,5;a\])",
      R"(<P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>there is </CHAR></TEXT><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="1" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="1"><SIZE Height="125" HeightRelTo="Absolute" Protect="false" Width="10" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>a+b</SCRIPT></EQUATION></TEXT><TEXT CharShape="1"><CHAR> something </CHAR></TEXT><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="2" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="2"><SIZE Height="10" HeightRelTo="Absolute" Protect="false" Width="5" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>a</SCRIPT></EQUATION></TEXT></P>)");
}

TEST(HwpTest, BoldAndItalic) {
  DoHwpTest(
      "**a** and some *bb* and text",
      R"(<P ParaShape="0" Style="0"><TEXT CharShape="7"><CHAR>**a**</CHAR></TEXT><TEXT CharShape="1"><CHAR> and some </CHAR></TEXT><TEXT CharShape="8"><CHAR>*bb*</CHAR></TEXT><TEXT CharShape="1"><CHAR> and text</CHAR></TEXT></P>)");
}

TEST(HwpTest, EscapedTab) {
  DoHwpTest(
      "① ㄱ\t② ㄷ",
      R"(<P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>① ㄱ<TAB/>② ㄷ</CHAR></TEXT></P>)");
}

TEST(HwpTest, Table) {
  DoHwpTest(
      R"(
|$$125,10;z$$|$$10,10;P(z)$$|
|---|---|
|0.1|123|
)",
      R"(</CHAR></TEXT></P><P ParaShape="0" Style="0"><TEXT CharShape="1"><TABLE BorderFill="6" CellSpacing="0" ColCount="2" PageBreak="Cell" RepeatHeader="true" RowCount="3"><SHAPEOBJECT InstId="1" Lock="false" NumberingType="Table" TextFlow="LargestOnly" ZOrder="1"><SIZE Height="5100" HeightRelTo="Absolute" Protect="false" Width="11000" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="850" Right="0" Top="0"/></SHAPEOBJECT><INSIDEMARGIN Bottom="141" Left="510" Right="510" Top="141"/><ROW><CELL BorderFill="11" ColAddr="0" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="1700" Protect="false" RowAddr="0" RowSpan="1" Width="5500"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center"><P ParaShape="0" Style="0"><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="2" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="2"><SIZE Height="125" HeightRelTo="Absolute" Protect="false" Width="10" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>z</SCRIPT></EQUATION></TEXT></P></PARALIST></CELL><CELL BorderFill="11" ColAddr="1" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="1700" Protect="false" RowAddr="0" RowSpan="1" Width="5500"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center"><P ParaShape="0" Style="0"><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="3" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="3"><SIZE Height="10" HeightRelTo="Absolute" Protect="false" Width="10" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>P(z)</SCRIPT></EQUATION></TEXT></P></PARALIST></CELL></ROW><ROW><CELL BorderFill="11" ColAddr="0" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="1700" Protect="false" RowAddr="1" RowSpan="1" Width="5500"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center"><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>0.1</CHAR></TEXT></P></PARALIST></CELL><CELL BorderFill="11" ColAddr="1" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="1700" Protect="false" RowAddr="1" RowSpan="1" Width="5500"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center"><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>123</CHAR></TEXT></P></PARALIST></CELL></ROW></TABLE></TEXT></P>)");
}

}  // namespace
}  // namespace md2


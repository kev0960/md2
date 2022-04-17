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

  HwpGenerator::HwpStatus hwp_status;
  GeneratorContext context(repo, "image_path", /*use_clang_server=*/false,
                           /*clang_server_port=*/0, nullptr, options);
  HwpGenerator generator(/*filename=*/"some_file.md", content, context, tree,
                         hwp_status);
  generator.Generate();

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
      R"(there is $$125,10;a+b$$ something \[10,5;a\])", R"(<P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>there is </CHAR></TEXT><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="1" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="1"><SIZE Height="125" HeightRelTo="Absolute" Protect="false" Width="10" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>a+b</SCRIPT></EQUATION></TEXT><TEXT CharShape="1"><CHAR> something </CHAR></TEXT><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="2" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="2"><SIZE Height="10" HeightRelTo="Absolute" Protect="false" Width="5" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>a</SCRIPT></EQUATION></TEXT></P>)");
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
      R"(<P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>
</CHAR></TEXT></P><P ParaShape="0" Style="0"><TEXT CharShape="1"><TABLE BorderFill="11" CellSpacing="0" ColCount="2" PageBreak="Cell" RepeatHeader="true" RowCount="2"><SHAPEOBJECT InstId="1" Lock="false" NumberingType="Table" TextFlow="LargestOnly" ZOrder="1"><SIZE Height="3400" HeightRelTo="Absolute" Protect="false" Width="42000" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="850" Right="0" Top="0"/></SHAPEOBJECT><INSIDEMARGIN Bottom="141" Left="510" Right="510" Top="141"/><ROW><CELL BorderFill="11" ColAddr="0" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="1700" Protect="false" RowAddr="0" RowSpan="1" Width="21000"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center"><P ParaShape="0" Style="0"><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="2" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="2"><SIZE Height="125" HeightRelTo="Absolute" Protect="false" Width="10" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>z</SCRIPT></EQUATION></TEXT></P></PARALIST></CELL><CELL BorderFill="11" ColAddr="1" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="1700" Protect="false" RowAddr="0" RowSpan="1" Width="21000"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center"><P ParaShape="0" Style="0"><TEXT CharShape="0"><EQUATION BaseLine="86" BaseUnit="1000" LineMode="false" TextColor="0" Version="Equation Version 60"><SHAPEOBJECT InstId="3" Lock="false" NumberingType="Equation" TextFlow="BothSides" ZOrder="3"><SIZE Height="10" HeightRelTo="Absolute" Protect="false" Width="10" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="56" Right="56" Top="0"/><SHAPECOMMENT>수식입니다.</SHAPECOMMENT></SHAPEOBJECT><SCRIPT>P(z)</SCRIPT></EQUATION></TEXT></P></PARALIST></CELL></ROW><ROW><CELL BorderFill="11" ColAddr="0" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="1700" Protect="false" RowAddr="1" RowSpan="1" Width="21000"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center"><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>0.1</CHAR></TEXT></P></PARALIST></CELL><CELL BorderFill="11" ColAddr="1" ColSpan="1" Dirty="false" Editable="false" HasMargin="false" Header="false" Height="1700" Protect="false" RowAddr="1" RowSpan="1" Width="21000"><CELLMARGIN Bottom="141" Left="510" Right="510" Top="141"/><PARALIST LineWrap="Break" LinkListID="0" LinkListIDNext="0" TextDirection="0" VertAlign="Center"><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>123</CHAR></TEXT></P></PARALIST></CELL></ROW></TABLE></TEXT></P>)");
}

TEST(HwpTest, Image) {
  DoHwpTest(
      "![](123,456)",
      R"(<P ParaShape="0" Style="0"><TEXT CharShape="0"><PICTURE Reverse="false"><SHAPEOBJECT InstId="1" Lock="false" NumberingType="Figure" ZOrder="1"><SIZE Height="123" HeightRelTo="Absolute" Protect="false" Width="456" WidthRelTo="Absolute"/><POSITION AffectLSpacing="false" AllowOverlap="false" FlowWithText="true" HoldAnchorAndSO="false" HorzAlign="Left" HorzOffset="0" HorzRelTo="Para" TreatAsChar="true" VertAlign="Top" VertOffset="0" VertRelTo="Para"/><OUTSIDEMARGIN Bottom="0" Left="0" Right="0" Top="0"/><SHAPECOMMENT></SHAPECOMMENT></SHAPEOBJECT><SHAPECOMPONENT CurHeight="123" CurWidth="456" GroupLevel="0" HorzFlip="false" InstID="2" OriHeight="123" OriWidth="456" VertFlip="false" XPos="0" YPos="0"><ROTATIONINFO Angle="0" CenterX="228" CenterY="61" Rotate="1"/><RENDERINGINFO><TRANSMATRIX E1="1.00000" E2="0.00000" E3="0.00000" E4="0.00000" E5="1.00000" E6="0.00000"/><SCAMATRIX E1="0.80000" E2="0.00000" E3="0.00000" E4="0.00000" E5="0.80000" E6="0.00000"/><ROTMATRIX E1="1.00000" E2="0.00000" E3="0.00000" E4="0.00000" E5="1.00000" E6="0.00000"/></RENDERINGINFO></SHAPECOMPONENT><IMAGERECT X0="0" X1="456" X2="456" X3="0" Y0="0" Y1="0" Y2="123" Y3="123"/><IMAGECLIP Bottom="123" Left="0" Right="456" Top="0"/><INSIDEMARGIN Bottom="0" Left="0" Right="0" Top="0"/><IMAGEDIM Height="123" Width="456"/><IMAGE Alpha="0" BinItem="1" Bright="0" Contrast="0" Effect="RealPic"/><EFFECTS/></PICTURE></TEXT></P>)");
}

TEST(HwpTest, Boxes) {
  std::string content = R"(
```candidates
* a

* b

* c
```
```examples
* x

* y

* z
```
)";

  DoHwpTest(content, R"(<P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>
</CHAR></TEXT></P><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>① </CHAR><CHAR>a</CHAR></TEXT><TEXT CharShape="1"><CHAR><TAB />② </CHAR><CHAR>b</CHAR></TEXT><TEXT CharShape="1"><CHAR><TAB />③ </CHAR><CHAR>c</CHAR></TEXT></P><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>
</CHAR></TEXT></P><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>ㄱ. </CHAR><CHAR>x</CHAR></TEXT></P><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>ㄴ. </CHAR><CHAR>y</CHAR></TEXT></P><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>ㄷ. </CHAR><CHAR>z</CHAR></TEXT></P><P ParaShape="0" Style="0"><TEXT CharShape="1"><CHAR>
</CHAR></TEXT></P>)");
}

TEST(HwpTest, ParagraphWithProblemStartConfig) {
  std::string content = "first\n\nsecond";
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  MetadataRepo repo;
  GeneratorOptions options;
  options.server_mode = true;

  HwpGenerator::HwpStatus hwp_status;
  GeneratorContext context(repo, "image_path", /*use_clang_server=*/false,
                           /*clang_server_port=*/0, nullptr, options);
  HwpGenerator generator(/*filename=*/"some_file.md", content, context, tree,
                         hwp_status);
  generator.GetHwpStateManager().AddParaShape(
      md2::HwpStateManager::HwpParaShapeType::PROBLEM_START_PARA, 10, 11);
  generator.GetHwpStateManager().AddTextShape(
      md2::HwpStateManager::HwpCharShapeType::PROBLEM_START_CHAR, 12);
  generator.Generate();

  EXPECT_EQ(std::string(generator.ShowOutput()),
            "<P ParaShape=\"10\" Style=\"11\"><TEXT "
            "CharShape=\"12\"><CHAR>first</CHAR></TEXT></P><P ParaShape=\"0\" "
            "Style=\"0\"><TEXT CharShape=\"1\"><CHAR>second</CHAR></TEXT></P>");
}

}  // namespace
}  // namespace md2


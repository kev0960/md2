#include "../src/parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "parse_tree_builder.h"

namespace md2 {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

void DoParserTest(std::string content, ParseTreeComparer comparer,
                  bool show_tree = false) {
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  if (show_tree) {
    tree.Print();
  }
  comparer.Compare(tree);
}

TEST(ParserTest, Paragraph) {
  Parser parser;

  std::string content = "a";

  ParseTree tree = parser.GenerateParseTree(content);
  ParseTreeComparer compare(
      {{ParseTreeNode::NODE, 0, 1, 0}, {ParseTreeNode::PARAGRAPH, 0, 1, 1}});
  compare.Compare(tree);
}

TEST(ParserTest, ParagraphLongerLines) {
  Parser parser;

  std::string content = "abc";

  ParseTree tree = parser.GenerateParseTree(content);
  ParseTreeComparer compare(
      {{ParseTreeNode::NODE, 0, 3, 0}, {ParseTreeNode::PARAGRAPH, 0, 3, 1}});
  compare.Compare(tree);
}

TEST(ParserTest, TwoParagraphs) {
  Parser parser;

  std::string content = "a\n\nb";

  ParseTree tree = parser.GenerateParseTree(content);
  ParseTreeComparer compare({{ParseTreeNode::NODE, 0, 4, 0},
                             {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                             {ParseTreeNode::PARAGRAPH, 3, 4, 1}});
  compare.Compare(tree);
}

TEST(ParserTest, BoldInParagraph) {
  Parser parser;

  std::string content = "a**b**c";

  ParseTree tree = parser.GenerateParseTree(content);
  ParseTreeComparer compare({{ParseTreeNode::NODE, 0, 7, 0},
                             {ParseTreeNode::PARAGRAPH, 0, 7, 1},
                             {ParseTreeNode::BOLD, 1, 6, 2}});
  compare.Compare(tree);
}

TEST(ParserTest, ItalicInParagraph) {
  Parser parser;

  std::string content = "a*b*c";

  ParseTree tree = parser.GenerateParseTree(content);
  ParseTreeComparer compare({{ParseTreeNode::NODE, 0, 5, 0},
                             {ParseTreeNode::PARAGRAPH, 0, 5, 1},
                             {ParseTreeNode::ITALIC, 1, 4, 2}});
  compare.Compare(tree);
}

TEST(ParserTest, ItalicInBold) {
  DoParserTest("***a***",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 7, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 7, 1},
                                  {ParseTreeNode::BOLD, 0, 7, 2},
                                  {ParseTreeNode::ITALIC, 2, 5, 3}}));
}

TEST(ParserTest, ItalicInBold2) {
  // a<b>b<i>c</i><i>d</i></b>
  DoParserTest("a**b*c**d***",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 12, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 12, 1},
                                  {ParseTreeNode::BOLD, 1, 12, 2},
                                  {ParseTreeNode::ITALIC, 4, 7, 3},
                                  {ParseTreeNode::ITALIC, 7, 10, 3}}));
}

TEST(ParserTest, ItalicInBold3) {
  // a<b>b<i>c</i><i>d</i></b>
  DoParserTest("a**b*c**d***",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 12, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 12, 1},
                                  {ParseTreeNode::BOLD, 1, 12, 2},
                                  {ParseTreeNode::ITALIC, 4, 7, 3},
                                  {ParseTreeNode::ITALIC, 7, 10, 3}}));
}

TEST(ParserTest, ItalicInBold4) {
  // a<b><i>b</i><i>c</i></b><b>a</b>
  DoParserTest("a***b**c*****a**",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 16, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 16, 1},
                                  {ParseTreeNode::BOLD, 1, 11, 2},
                                  {ParseTreeNode::ITALIC, 3, 6, 3},
                                  {ParseTreeNode::ITALIC, 6, 9, 3},
                                  {ParseTreeNode::BOLD, 11, 16, 2}}));
}

TEST(ParserTest, BoldInItalic) {
  // <i>a<b>b</b></i>
  DoParserTest("*a**b***",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 8, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 8, 1},
                                  {ParseTreeNode::ITALIC, 0, 8, 2},
                                  {ParseTreeNode::BOLD, 2, 7, 3}}));
}

TEST(ParserTest, BoldInItalic2) {
  // <i>a<b>b</b><b>c</b></i><b>e</b>
  DoParserTest("*a**b****c***",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 13, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 13, 1},
                                  {ParseTreeNode::ITALIC, 0, 13, 2},
                                  {ParseTreeNode::BOLD, 2, 7, 3},
                                  {ParseTreeNode::BOLD, 7, 12, 3}}));
}

TEST(ParserTest, Link) {
  DoParserTest("[link](http://link)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 19, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 19, 1},
                                  {ParseTreeNode::LINK, 0, 19, 2},
                                  {ParseTreeNode::NODE, 0, 6, 3},
                                  {ParseTreeNode::PARAGRAPH, 1, 5, 4},
                                  {ParseTreeNode::NODE, 6, 19, 3},
                                  {ParseTreeNode::PARAGRAPH, 7, 18, 4}}));
}

TEST(ParserTest, Link2) {
  // "]" inside of the ** should be ignored.
  DoParserTest("[link *]*](http://link)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 23, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 23, 1},
                                  {ParseTreeNode::LINK, 0, 23, 2},
                                  {ParseTreeNode::NODE, 0, 10, 3},
                                  {ParseTreeNode::PARAGRAPH, 1, 9, 4},
                                  {ParseTreeNode::ITALIC, 6, 9, 5},
                                  {ParseTreeNode::NODE, 10, 23, 3},
                                  {ParseTreeNode::PARAGRAPH, 11, 22, 4}}));
}

TEST(ParserTest, Link3) {
  DoParserTest("arr[0][1][2](link)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 18, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 18, 1},
                                  {ParseTreeNode::LINK, 9, 18, 2},
                                  {ParseTreeNode::NODE, 9, 12, 3},
                                  {ParseTreeNode::PARAGRAPH, 10, 11, 4},
                                  {ParseTreeNode::NODE, 12, 18, 3},
                                  {ParseTreeNode::PARAGRAPH, 13, 17, 4}}));
}

TEST(ParserTest, InvalidLink) {
  DoParserTest("[link] (http://link)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 20, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 20, 1}}));
}

TEST(ParserTest, InvalidLink2) {
  DoParserTest("arr[0][1][2]",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 12, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 12, 1}}));
}

TEST(ParserTest, EscapeChar) {
  // \b and \note are not escaped.
  DoParserTest(R"(\*\b\\\note)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 11, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 11, 1},
                                  {ParseTreeNode::ESCAPE, 0, 2, 2},
                                  {ParseTreeNode::ESCAPE, 4, 6, 2}}));
}

TEST(ParserTest, Image) {
  DoParserTest("![alttext](http://img)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 22, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 22, 1},
                                  {ParseTreeNode::IMAGE, 1, 22, 2},
                                  {ParseTreeNode::NODE, 1, 10, 3},
                                  {ParseTreeNode::PARAGRAPH, 2, 9, 4},
                                  {ParseTreeNode::TEXT, 2, 9, 5},  // Alt text
                                  {ParseTreeNode::NODE, 10, 22, 3},
                                  {ParseTreeNode::PARAGRAPH, 11, 21, 4}}));
}

TEST(ParserTest, ImageWithCaption) {
  DoParserTest("![alttext caption=abc](http://img)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 34, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 34, 1},
                                  {ParseTreeNode::IMAGE, 1, 34, 2},
                                  {ParseTreeNode::NODE, 1, 22, 3},
                                  {ParseTreeNode::PARAGRAPH, 2, 21, 4},
                                  {ParseTreeNode::TEXT, 2, 10, 5},   // Alt text
                                  {ParseTreeNode::TEXT, 18, 21, 5},  // Caption
                                  {ParseTreeNode::NODE, 22, 34, 3},
                                  {ParseTreeNode::PARAGRAPH, 23, 33, 4}}));
}

TEST(ParserTest, ImageWithCaptionBold) {
  DoParserTest(
      "![alttext caption=some**aa**](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 41, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 41, 1},
                         {ParseTreeNode::IMAGE, 1, 41, 2},
                         {ParseTreeNode::NODE, 1, 29, 3},
                         {ParseTreeNode::PARAGRAPH, 2, 28, 4},
                         {ParseTreeNode::TEXT, 2, 10, 5},   // Alt text
                         {ParseTreeNode::TEXT, 18, 28, 5},  // Caption
                         {ParseTreeNode::BOLD, 22, 28, 6},  // Caption BOLD
                         {ParseTreeNode::NODE, 29, 41, 3},
                         {ParseTreeNode::PARAGRAPH, 30, 40, 4}}));
}

TEST(ParserTest, ImageWithCaptionBold2) {
  // "caption=" inside of ** should be ignored.
  DoParserTest(
      "![alttext caption=some**caption=**](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 47, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 47, 1},
                         {ParseTreeNode::IMAGE, 1, 47, 2},
                         {ParseTreeNode::NODE, 1, 35, 3},
                         {ParseTreeNode::PARAGRAPH, 2, 34, 4},
                         {ParseTreeNode::TEXT, 2, 10, 5},   // Alt text
                         {ParseTreeNode::TEXT, 18, 34, 5},  // Caption
                         {ParseTreeNode::BOLD, 22, 34, 6},  // Caption BOLD
                         {ParseTreeNode::NODE, 35, 47, 3},
                         {ParseTreeNode::PARAGRAPH, 36, 46, 4}}));
}

TEST(ParserTest, ImageWithCaptionComplex) {
  // "caption=" inside of ** should be ignored.
  DoParserTest(
      "![alttext caption=some**cap***a*](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 45, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 45, 1},
                         {ParseTreeNode::IMAGE, 1, 45, 2},
                         {ParseTreeNode::NODE, 1, 33, 3},
                         {ParseTreeNode::PARAGRAPH, 2, 32, 4},
                         {ParseTreeNode::TEXT, 2, 10, 5},     // Alt text
                         {ParseTreeNode::TEXT, 18, 32, 5},    // Caption
                         {ParseTreeNode::BOLD, 22, 29, 6},    // Caption BOLD
                         {ParseTreeNode::ITALIC, 29, 32, 6},  // Caption Italic
                         {ParseTreeNode::NODE, 33, 45, 3},
                         {ParseTreeNode::PARAGRAPH, 34, 44, 4}}));
}

TEST(ParserTest, ImageWithCaptionAndSize) {
  // "caption=" inside of ** should be ignored.
  DoParserTest("![alttext caption=cap size=123x123](http://img)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 47, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 47, 1},
                                  {ParseTreeNode::IMAGE, 1, 47, 2},
                                  {ParseTreeNode::NODE, 1, 35, 3},
                                  {ParseTreeNode::PARAGRAPH, 2, 34, 4},
                                  {ParseTreeNode::TEXT, 2, 10, 5},   // Alt text
                                  {ParseTreeNode::TEXT, 18, 22, 5},  // Caption
                                  {ParseTreeNode::TEXT, 27, 34, 5},  // Size
                                  {ParseTreeNode::NODE, 35, 47, 3},
                                  {ParseTreeNode::PARAGRAPH, 36, 46, 4}}));
}

TEST(ParserTest, ImageWithCaptionItalicAndSize) {
  // "caption=" inside of ** should be ignored.
  DoParserTest(
      "![alttext caption=*a* size=123x123](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 47, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 47, 1},
                         {ParseTreeNode::IMAGE, 1, 47, 2},
                         {ParseTreeNode::NODE, 1, 35, 3},
                         {ParseTreeNode::PARAGRAPH, 2, 34, 4},
                         {ParseTreeNode::TEXT, 2, 10, 5},     // Alt text
                         {ParseTreeNode::TEXT, 18, 22, 5},    // Caption
                         {ParseTreeNode::ITALIC, 18, 21, 6},  // Caption Italic
                         {ParseTreeNode::TEXT, 27, 34, 5},    // Size
                         {ParseTreeNode::NODE, 35, 47, 3},
                         {ParseTreeNode::PARAGRAPH, 36, 46, 4}}));
}

TEST(ParserTest, ImageWithCaptionItalicAndSizeNoAlt) {
  // "caption=" inside of ** should be ignored.
  DoParserTest(
      "![caption=*a* size=123x123](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 39, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 39, 1},
                         {ParseTreeNode::IMAGE, 1, 39, 2},
                         {ParseTreeNode::NODE, 1, 27, 3},
                         {ParseTreeNode::PARAGRAPH, 2, 26, 4},
                         {ParseTreeNode::TEXT, 2, 2, 5},    // Alt Text (empty)
                         {ParseTreeNode::TEXT, 10, 14, 5},  // Caption
                         {ParseTreeNode::ITALIC, 10, 13, 6},  // Caption Italic
                         {ParseTreeNode::TEXT, 19, 26, 5},    // Size
                         {ParseTreeNode::NODE, 27, 39, 3},
                         {ParseTreeNode::PARAGRAPH, 28, 38, 4}}));
}

TEST(ParserTest, HeaderSimple) {
  DoParserTest("### header", ParseTreeComparer({
                                 {ParseTreeNode::NODE, 0, 10, 0},
                                 {ParseTreeNode::HEADER, 0, 10, 1},
                                 {ParseTreeNode::TEXT, 0, 3, 2},
                                 {ParseTreeNode::TEXT, 3, 10, 2},
                                 {ParseTreeNode::PARAGRAPH, 10, 10, 1},
                             }));
}

TEST(ParserTest, HeaderSimple2) {
  DoParserTest("a\n### header", ParseTreeComparer({
                                    {ParseTreeNode::NODE, 0, 12, 0},
                                    {ParseTreeNode::PARAGRAPH, 0, 2, 1},
                                    {ParseTreeNode::HEADER, 2, 12, 1},
                                    {ParseTreeNode::TEXT, 2, 5, 2},
                                    {ParseTreeNode::TEXT, 5, 12, 2},
                                    {ParseTreeNode::PARAGRAPH, 12, 12, 1},
                                }));
}

TEST(ParserTest, NotHeader) {
  DoParserTest("a### header",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 11, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 11, 1}}));
}

TEST(ParserTest, SimpleVerbatim) {
  DoParserTest("a `code` a",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 10, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 10, 1},
                                  {ParseTreeNode::VERBATIM, 2, 8, 2}}));
}

TEST(ParserTest, SimpleCode) {
  DoParserTest(R"(
```cpp
hello;
```)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 18, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::VERBATIM, 1, 18, 1},
                                  {ParseTreeNode::TEXT, 4, 7, 2},
                                  {ParseTreeNode::TEXT, 8, 15, 2},
                                  {ParseTreeNode::PARAGRAPH, 18, 18, 1}}));
}

TEST(ParserTest, NestedBox) {
  std::string content = R"(
```box
hello;
```box
*a*
```
b
```)";

  DoParserTest(
      content,
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 35, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                         {ParseTreeNode::BOX, 1, 35, 1},
                         {ParseTreeNode::TEXT, 4, 7, 2},  // Box name
                         {ParseTreeNode::NODE, 7, 35, 2},
                         {ParseTreeNode::PARAGRAPH, 7, 15, 3},  // "\nhello;\n"
                         {ParseTreeNode::BOX, 15, 29, 3},
                         {ParseTreeNode::TEXT, 18, 21, 4},  // Box name
                         {ParseTreeNode::NODE, 21, 29, 4},
                         {ParseTreeNode::PARAGRAPH, 21, 26, 5},  // "\n*a*\n"
                         {ParseTreeNode::ITALIC, 22, 25, 6},
                         {ParseTreeNode::PARAGRAPH, 29, 32, 3},  // "\nb\n"
                         {ParseTreeNode::PARAGRAPH, 35, 35, 1}}));
}

TEST(ParserTest, SimpleTable) {
  std::string content = R"(
|a|b|c|
|-|-|-|
|a|b|c|
abc)";

  DoParserTest(content,
               ParseTreeComparer({
                   {ParseTreeNode::NODE, 0, 28, 0},
                   {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                   {ParseTreeNode::TABLE, 1, 25, 1},
                   {ParseTreeNode::NODE, 2, 4, 2},  // Contains the ending '|'
                   {ParseTreeNode::PARAGRAPH, 2, 3, 3},
                   {ParseTreeNode::NODE, 4, 6, 2},
                   {ParseTreeNode::PARAGRAPH, 4, 5, 3},
                   {ParseTreeNode::NODE, 6, 8, 2},
                   {ParseTreeNode::PARAGRAPH, 6, 7, 3},
                   {ParseTreeNode::NODE, 10, 12, 2},
                   {ParseTreeNode::PARAGRAPH, 10, 11, 3},
                   {ParseTreeNode::NODE, 12, 14, 2},
                   {ParseTreeNode::PARAGRAPH, 12, 13, 3},
                   {ParseTreeNode::NODE, 14, 16, 2},
                   {ParseTreeNode::PARAGRAPH, 14, 15, 3},
                   {ParseTreeNode::NODE, 18, 20, 2},
                   {ParseTreeNode::PARAGRAPH, 18, 19, 3},
                   {ParseTreeNode::NODE, 20, 22, 2},
                   {ParseTreeNode::PARAGRAPH, 20, 21, 3},
                   {ParseTreeNode::NODE, 22, 24, 2},
                   {ParseTreeNode::PARAGRAPH, 22, 23, 3},
                   {ParseTreeNode::PARAGRAPH, 25, 28, 1},
               }));
}

TEST(ParserTest, SimpleTable2) {
  std::string content = R"(
|*a*|**b**|
|-|-|
|\|a|`b`|)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 28, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::TABLE, 1, 28, 1},
                                  {ParseTreeNode::NODE, 2, 6, 2},
                                  {ParseTreeNode::PARAGRAPH, 2, 5, 3},
                                  {ParseTreeNode::ITALIC, 2, 5, 4},
                                  {ParseTreeNode::NODE, 6, 12, 2},
                                  {ParseTreeNode::PARAGRAPH, 6, 11, 3},
                                  {ParseTreeNode::BOLD, 6, 11, 4},
                                  {ParseTreeNode::NODE, 14, 16, 2},
                                  {ParseTreeNode::PARAGRAPH, 14, 15, 3},
                                  {ParseTreeNode::NODE, 16, 18, 2},
                                  {ParseTreeNode::PARAGRAPH, 16, 17, 3},
                                  {ParseTreeNode::NODE, 20, 24, 2},
                                  {ParseTreeNode::PARAGRAPH, 20, 23, 3},
                                  {ParseTreeNode::ESCAPE, 20, 22, 4},
                                  {ParseTreeNode::NODE, 24, 28, 2},
                                  {ParseTreeNode::PARAGRAPH, 24, 27, 3},
                                  {ParseTreeNode::VERBATIM, 24, 27, 4},
                                  {ParseTreeNode::PARAGRAPH, 28, 28, 1}}));
}

}  // namespace
}  // namespace md2

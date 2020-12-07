#include "../src/parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "logger.h"
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

TEST(ParserTest, StrikeThrough) {
  DoParserTest("~~ab~~",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 6, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 6, 1},
                                  {ParseTreeNode::STRIKE_THROUGH, 0, 6, 2}}));
}

TEST(ParserTest, StrikeThrough2) {
  DoParserTest("a~~a*b*~~c",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 10, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 10, 1},
                                  {ParseTreeNode::STRIKE_THROUGH, 1, 9, 2},
                                  {ParseTreeNode::ITALIC, 4, 7, 3}}));
}

TEST(ParserTest, Link) {
  DoParserTest("[link](http://link)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 19, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 19, 1},
                                  {ParseTreeNode::LINK, 0, 19, 2},
                                  {ParseTreeNode::NODE, 0, 6, 3},
                                  {ParseTreeNode::TEXT, 1, 5, 4},
                                  {ParseTreeNode::NODE, 6, 19, 3},
                                  {ParseTreeNode::TEXT, 7, 18, 4}}));
}

TEST(ParserTest, Link2) {
  // "]" inside of the ** should be ignored.
  DoParserTest("[link *]*](http://link)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 23, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 23, 1},
                                  {ParseTreeNode::LINK, 0, 23, 2},
                                  {ParseTreeNode::NODE, 0, 10, 3},
                                  {ParseTreeNode::TEXT, 1, 9, 4},
                                  {ParseTreeNode::ITALIC, 6, 9, 5},
                                  {ParseTreeNode::NODE, 10, 23, 3},
                                  {ParseTreeNode::TEXT, 11, 22, 4}}));
}

TEST(ParserTest, Link3) {
  DoParserTest("arr[0][1][2](link)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 18, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 18, 1},
                                  {ParseTreeNode::LINK, 9, 18, 2},
                                  {ParseTreeNode::NODE, 9, 12, 3},
                                  {ParseTreeNode::TEXT, 10, 11, 4},
                                  {ParseTreeNode::NODE, 12, 18, 3},
                                  {ParseTreeNode::TEXT, 13, 17, 4}}));
}

TEST(ParserTest, Link4) {
  DoParserTest("some [arr[2[3 blah](abc)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 24, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 24, 1},
                                  {ParseTreeNode::LINK, 5, 24, 2},
                                  {ParseTreeNode::NODE, 5, 19, 3},
                                  {ParseTreeNode::TEXT, 6, 18, 4},
                                  {ParseTreeNode::NODE, 19, 24, 3},
                                  {ParseTreeNode::TEXT, 20, 23, 4}}));
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
                                  {ParseTreeNode::IMAGE, 0, 22, 2},
                                  {ParseTreeNode::NODE, 0, 10, 3},
                                  {ParseTreeNode::TEXT, 2, 9, 4},
                                  {ParseTreeNode::TEXT, 2, 9, 5},  // Alt text
                                  {ParseTreeNode::NODE, 10, 22, 3},
                                  {ParseTreeNode::TEXT, 11, 21, 4}}));
}

TEST(ParserTest, ImageWithCaption) {
  DoParserTest("![alttext caption=abc](http://img)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 34, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 34, 1},
                                  {ParseTreeNode::IMAGE, 0, 34, 2},
                                  {ParseTreeNode::NODE, 0, 22, 3},
                                  {ParseTreeNode::TEXT, 2, 21, 4},
                                  {ParseTreeNode::TEXT, 2, 10, 5},   // Alt text
                                  {ParseTreeNode::TEXT, 18, 21, 5},  // Caption
                                  {ParseTreeNode::NODE, 22, 34, 3},
                                  {ParseTreeNode::TEXT, 23, 33, 4}}));
}

TEST(ParserTest, ImageWithCaptionBold) {
  DoParserTest(
      "![alttext caption=some**aa**](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 41, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 41, 1},
                         {ParseTreeNode::IMAGE, 0, 41, 2},
                         {ParseTreeNode::NODE, 0, 29, 3},
                         {ParseTreeNode::TEXT, 2, 28, 4},
                         {ParseTreeNode::TEXT, 2, 10, 5},   // Alt text
                         {ParseTreeNode::TEXT, 18, 28, 5},  // Caption
                         {ParseTreeNode::BOLD, 22, 28, 6},  // Caption BOLD
                         {ParseTreeNode::NODE, 29, 41, 3},
                         {ParseTreeNode::TEXT, 30, 40, 4}}));
}

TEST(ParserTest, ImageWithCaptionBold2) {
  // "caption=" inside of ** should be ignored.
  DoParserTest(
      "![alttext caption=some**caption=**](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 47, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 47, 1},
                         {ParseTreeNode::IMAGE, 0, 47, 2},
                         {ParseTreeNode::NODE, 0, 35, 3},
                         {ParseTreeNode::TEXT, 2, 34, 4},
                         {ParseTreeNode::TEXT, 2, 10, 5},   // Alt text
                         {ParseTreeNode::TEXT, 18, 34, 5},  // Caption
                         {ParseTreeNode::BOLD, 22, 34, 6},  // Caption BOLD
                         {ParseTreeNode::NODE, 35, 47, 3},
                         {ParseTreeNode::TEXT, 36, 46, 4}}));
}

TEST(ParserTest, ImageWithCaptionComplex) {
  // "caption=" inside of ** should be ignored.
  DoParserTest(
      "![alttext caption=some**cap***a*](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 45, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 45, 1},
                         {ParseTreeNode::IMAGE, 0, 45, 2},
                         {ParseTreeNode::NODE, 0, 33, 3},
                         {ParseTreeNode::TEXT, 2, 32, 4},
                         {ParseTreeNode::TEXT, 2, 10, 5},     // Alt text
                         {ParseTreeNode::TEXT, 18, 32, 5},    // Caption
                         {ParseTreeNode::BOLD, 22, 29, 6},    // Caption BOLD
                         {ParseTreeNode::ITALIC, 29, 32, 6},  // Caption Italic
                         {ParseTreeNode::NODE, 33, 45, 3},
                         {ParseTreeNode::TEXT, 34, 44, 4}}));
}

TEST(ParserTest, ImageWithCaptionAndSize) {
  // "caption=" inside of ** should be ignored.
  DoParserTest("![alttext caption=cap size=123x123](http://img)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 47, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 47, 1},
                                  {ParseTreeNode::IMAGE, 0, 47, 2},
                                  {ParseTreeNode::NODE, 0, 35, 3},
                                  {ParseTreeNode::TEXT, 2, 34, 4},
                                  {ParseTreeNode::TEXT, 2, 10, 5},   // Alt text
                                  {ParseTreeNode::TEXT, 18, 22, 5},  // Caption
                                  {ParseTreeNode::TEXT, 27, 34, 5},  // Size
                                  {ParseTreeNode::NODE, 35, 47, 3},
                                  {ParseTreeNode::TEXT, 36, 46, 4}}));
}

TEST(ParserTest, ImageWithCaptionItalicAndSize) {
  // "caption=" inside of ** should be ignored.
  DoParserTest(
      "![alttext caption=*a* size=123x123](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 47, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 47, 1},
                         {ParseTreeNode::IMAGE, 0, 47, 2},
                         {ParseTreeNode::NODE, 0, 35, 3},
                         {ParseTreeNode::TEXT, 2, 34, 4},
                         {ParseTreeNode::TEXT, 2, 10, 5},     // Alt text
                         {ParseTreeNode::TEXT, 18, 22, 5},    // Caption
                         {ParseTreeNode::ITALIC, 18, 21, 6},  // Caption Italic
                         {ParseTreeNode::TEXT, 27, 34, 5},    // Size
                         {ParseTreeNode::NODE, 35, 47, 3},
                         {ParseTreeNode::TEXT, 36, 46, 4}}));
}

TEST(ParserTest, ImageWithCaptionItalicAndSizeNoAlt) {
  // "caption=" inside of ** should be ignored.
  DoParserTest(
      "![caption=*a* size=123x123](http://img)",
      ParseTreeComparer({{ParseTreeNode::NODE, 0, 39, 0},
                         {ParseTreeNode::PARAGRAPH, 0, 39, 1},
                         {ParseTreeNode::IMAGE, 0, 39, 2},
                         {ParseTreeNode::NODE, 0, 27, 3},
                         {ParseTreeNode::TEXT, 2, 26, 4},
                         {ParseTreeNode::TEXT, 2, 2, 5},    // Alt Text (empty)
                         {ParseTreeNode::TEXT, 10, 14, 5},  // Caption
                         {ParseTreeNode::ITALIC, 10, 13, 6},  // Caption Italic
                         {ParseTreeNode::TEXT, 19, 26, 5},    // Size
                         {ParseTreeNode::NODE, 27, 39, 3},
                         {ParseTreeNode::TEXT, 28, 38, 4}}));
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

TEST(ParserTest, LectureHeader) {
  DoParserTest("###@ [Some](lecture)",
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 20, 0},
                                  {ParseTreeNode::HEADER, 0, 20, 1},
                                  {ParseTreeNode::TEXT, 0, 4, 2},
                                  {ParseTreeNode::NODE, 4, 20, 2},
                                  {ParseTreeNode::TEXT, 4, 20, 3},
                                  {ParseTreeNode::LINK, 5, 20, 4},
                                  {ParseTreeNode::NODE, 5, 11, 5},
                                  {ParseTreeNode::TEXT, 6, 10, 6},
                                  {ParseTreeNode::NODE, 11, 20, 5},
                                  {ParseTreeNode::TEXT, 12, 19, 6},
                                  {ParseTreeNode::PARAGRAPH, 20, 20, 1}}));
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
                         {ParseTreeNode::NODE, 8, 35, 2},
                         {ParseTreeNode::PARAGRAPH, 8, 15, 3},  // "hello;\n"
                         {ParseTreeNode::BOX, 15, 29, 3},
                         {ParseTreeNode::TEXT, 18, 21, 4},  // Box name
                         {ParseTreeNode::NODE, 22, 29, 4},
                         {ParseTreeNode::PARAGRAPH, 22, 25, 5},  // "*a*"
                         {ParseTreeNode::ITALIC, 22, 25, 6},
                         {ParseTreeNode::PARAGRAPH, 29, 31, 3},  // "\nb"
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

TEST(ParserTest, SimpleList) {
  std::string content = R"(
* a
* b
  c
* d)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 16, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::LIST, 1, 16, 1},
                                  {ParseTreeNode::LIST_ITEM, 1, 5, 2},
                                  {ParseTreeNode::PARAGRAPH, 3, 4, 3},
                                  {ParseTreeNode::LIST_ITEM, 5, 13, 2},
                                  {ParseTreeNode::PARAGRAPH, 7, 8, 3},
                                  {ParseTreeNode::PARAGRAPH, 9, 12, 3},
                                  {ParseTreeNode::LIST_ITEM, 13, 16, 2},
                                  {ParseTreeNode::PARAGRAPH, 15, 16, 3},
                                  {ParseTreeNode::PARAGRAPH, 16, 16, 1}}));
}

TEST(ParserTest, SimpleNestedList) {
  std::string content = R"(
* a
  * b
    * c
    * d
* e)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 30, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::LIST, 1, 30, 1},
                                  {ParseTreeNode::LIST_ITEM, 1, 5, 2},
                                  {ParseTreeNode::PARAGRAPH, 3, 4, 3},
                                  {ParseTreeNode::LIST, 5, 27, 2},
                                  {ParseTreeNode::LIST_ITEM, 5, 11, 3},
                                  {ParseTreeNode::PARAGRAPH, 9, 10, 4},
                                  {ParseTreeNode::LIST, 11, 27, 3},
                                  {ParseTreeNode::LIST_ITEM, 11, 19, 4},
                                  {ParseTreeNode::PARAGRAPH, 17, 18, 5},
                                  {ParseTreeNode::LIST_ITEM, 19, 27, 4},
                                  {ParseTreeNode::PARAGRAPH, 25, 26, 5},
                                  {ParseTreeNode::LIST_ITEM, 27, 30, 2},
                                  {ParseTreeNode::PARAGRAPH, 29, 30, 3},
                                  {ParseTreeNode::PARAGRAPH, 30, 30, 1}}));
}

TEST(ParserTest, SimpleNestedListWithSomeText) {
  std::string content = R"(
* a
  a2
  * b1
    * c1
    * d1
  * b2
    b3
    * c2
* e)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 61, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::LIST, 1, 61, 1},
                                  {ParseTreeNode::LIST_ITEM, 1, 10, 2},
                                  {ParseTreeNode::PARAGRAPH, 3, 4, 3},
                                  {ParseTreeNode::PARAGRAPH, 5, 9, 3},
                                  {ParseTreeNode::LIST, 10, 58, 2},
                                  {ParseTreeNode::LIST_ITEM, 10, 17, 3},
                                  {ParseTreeNode::PARAGRAPH, 14, 16, 4},
                                  {ParseTreeNode::LIST, 17, 35, 3},
                                  {ParseTreeNode::LIST_ITEM, 17, 26, 4},
                                  {ParseTreeNode::PARAGRAPH, 23, 25, 5},
                                  {ParseTreeNode::LIST_ITEM, 26, 35, 4},
                                  {ParseTreeNode::PARAGRAPH, 32, 34, 5},
                                  {ParseTreeNode::LIST_ITEM, 35, 49, 3},
                                  {ParseTreeNode::PARAGRAPH, 39, 41, 4},
                                  {ParseTreeNode::PARAGRAPH, 42, 48, 4},
                                  {ParseTreeNode::LIST, 49, 58, 3},
                                  {ParseTreeNode::LIST_ITEM, 49, 58, 4},
                                  {ParseTreeNode::PARAGRAPH, 55, 57, 5},
                                  {ParseTreeNode::LIST_ITEM, 58, 61, 2},
                                  {ParseTreeNode::PARAGRAPH, 60, 61, 3},
                                  {ParseTreeNode::PARAGRAPH, 61, 61, 1}}));
}

TEST(ParserTest, ParagraphMiddleOfList) {
  std::string content = R"(
* a

some text
* e)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 19, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::LIST, 1, 6, 1},
                                  {ParseTreeNode::LIST_ITEM, 1, 6, 2},
                                  {ParseTreeNode::PARAGRAPH, 3, 4, 3},
                                  {ParseTreeNode::PARAGRAPH, 6, 16, 1},
                                  {ParseTreeNode::LIST, 16, 19, 1},
                                  {ParseTreeNode::LIST_ITEM, 16, 19, 2},
                                  {ParseTreeNode::PARAGRAPH, 18, 19, 3},
                                  {ParseTreeNode::PARAGRAPH, 19, 19, 1}}));
}

TEST(ParserTest, ParagraphMiddleOfListWithLongEmptyNewline) {
  // Note the empty line above some text is "   \n", not just "\n".
  std::string content = R"(
* a
   
some text
* e)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 22, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::LIST, 1, 9, 1},
                                  {ParseTreeNode::LIST_ITEM, 1, 9, 2},
                                  {ParseTreeNode::PARAGRAPH, 3, 4, 3},
                                  {ParseTreeNode::PARAGRAPH, 9, 19, 1},
                                  {ParseTreeNode::LIST, 19, 22, 1},
                                  {ParseTreeNode::LIST_ITEM, 19, 22, 2},
                                  {ParseTreeNode::PARAGRAPH, 21, 22, 3},
                                  {ParseTreeNode::PARAGRAPH, 22, 22, 1}}));
}

TEST(ParserTest, SimpleListWithVerbatim) {
  std::string content = R"(
* code
```cpp
abc
```
* code2 
```cpp
def
```)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 46, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::LIST, 1, 46, 1},
                                  {ParseTreeNode::LIST_ITEM, 1, 23, 2},
                                  {ParseTreeNode::PARAGRAPH, 3, 7, 3},
                                  {ParseTreeNode::VERBATIM, 8, 22, 3},
                                  {ParseTreeNode::TEXT, 11, 14, 4},
                                  {ParseTreeNode::TEXT, 15, 19, 4},
                                  {ParseTreeNode::PARAGRAPH, 22, 22, 3},
                                  {ParseTreeNode::LIST_ITEM, 23, 46, 2},
                                  {ParseTreeNode::PARAGRAPH, 25, 31, 3},
                                  {ParseTreeNode::VERBATIM, 32, 46, 3},
                                  {ParseTreeNode::TEXT, 35, 38, 4},
                                  {ParseTreeNode::TEXT, 39, 43, 4},
                                  {ParseTreeNode::PARAGRAPH, 46, 46, 3},
                                  {ParseTreeNode::PARAGRAPH, 46, 46, 1}}));
}

TEST(ParserTest, SimpleOrderedList) {
  std::string content = R"(
1. a
2. b
  c
3. d)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 19, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::ORDERED_LIST, 1, 19, 1},
                                  {ParseTreeNode::ORDERED_LIST_ITEM, 1, 6, 2},
                                  {ParseTreeNode::PARAGRAPH, 4, 5, 3},
                                  {ParseTreeNode::ORDERED_LIST_ITEM, 6, 15, 2},
                                  {ParseTreeNode::PARAGRAPH, 9, 10, 3},
                                  {ParseTreeNode::PARAGRAPH, 11, 14, 3},
                                  {ParseTreeNode::ORDERED_LIST_ITEM, 15, 19, 2},
                                  {ParseTreeNode::PARAGRAPH, 18, 19, 3},
                                  {ParseTreeNode::PARAGRAPH, 19, 19, 1}}));
}

TEST(ParserTest, OrderedAndUnorderedMixed) {
  std::string content = R"(
1. a
2. b
  * c
  * d
    1. e
    2. f
  * g
    * a
3. d)";

  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 59, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                                  {ParseTreeNode::ORDERED_LIST, 1, 59, 1},
                                  {ParseTreeNode::ORDERED_LIST_ITEM, 1, 6, 2},
                                  {ParseTreeNode::PARAGRAPH, 4, 5, 3},
                                  {ParseTreeNode::ORDERED_LIST_ITEM, 6, 11, 2},
                                  {ParseTreeNode::PARAGRAPH, 9, 10, 3},
                                  {ParseTreeNode::LIST, 11, 55, 2},
                                  {ParseTreeNode::LIST_ITEM, 11, 17, 3},
                                  {ParseTreeNode::PARAGRAPH, 15, 16, 4},
                                  {ParseTreeNode::LIST_ITEM, 17, 23, 3},
                                  {ParseTreeNode::PARAGRAPH, 21, 22, 4},
                                  {ParseTreeNode::ORDERED_LIST, 23, 41, 3},
                                  {ParseTreeNode::ORDERED_LIST_ITEM, 23, 32, 4},
                                  {ParseTreeNode::PARAGRAPH, 30, 31, 5},
                                  {ParseTreeNode::ORDERED_LIST_ITEM, 32, 41, 4},
                                  {ParseTreeNode::PARAGRAPH, 39, 40, 5},
                                  {ParseTreeNode::LIST_ITEM, 41, 47, 3},
                                  {ParseTreeNode::PARAGRAPH, 45, 46, 4},
                                  {ParseTreeNode::LIST, 47, 55, 3},
                                  {ParseTreeNode::LIST_ITEM, 47, 55, 4},
                                  {ParseTreeNode::PARAGRAPH, 53, 54, 5},
                                  {ParseTreeNode::ORDERED_LIST_ITEM, 55, 59, 2},
                                  {ParseTreeNode::PARAGRAPH, 58, 59, 3},
                                  {ParseTreeNode::PARAGRAPH, 59, 59, 1}}));
}

TEST(ParserTest, SimpleCommand) {
  std::string content = R"(\sidenote{this *is* a sidenote})";
  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 31, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 31, 1},
                                  {ParseTreeNode::COMMAND, 0, 31, 2},
                                  {ParseTreeNode::NODE, 10, 30, 3},
                                  {ParseTreeNode::TEXT, 10, 30, 4},
                                  {ParseTreeNode::ITALIC, 15, 19, 5}}));
}

TEST(ParserTest, MultipleArgCommand) {
  std::string content = R"(\tooltip{**a**}{some `code`})";
  DoParserTest(content, ParseTreeComparer({{ParseTreeNode::NODE, 0, 28, 0},
                                           {ParseTreeNode::PARAGRAPH, 0, 28, 1},
                                           {ParseTreeNode::COMMAND, 0, 28, 2},
                                           {ParseTreeNode::NODE, 9, 14, 3},
                                           {ParseTreeNode::TEXT, 9, 14, 4},
                                           {ParseTreeNode::BOLD, 9, 14, 5},
                                           {ParseTreeNode::TEXT, 16, 27, 3}}));
}

TEST(ParserTest, CommandAndInvalidCommand) {
  std::string content = R"(\tooltip{a}{b} \tooltip{a})";
  DoParserTest(content, ParseTreeComparer({{ParseTreeNode::NODE, 0, 26, 0},
                                           {ParseTreeNode::PARAGRAPH, 0, 26, 1},
                                           {ParseTreeNode::COMMAND, 0, 14, 2},
                                           {ParseTreeNode::NODE, 9, 10, 3},
                                           {ParseTreeNode::TEXT, 9, 10, 4},
                                           {ParseTreeNode::TEXT, 12, 13, 3}}));
}

TEST(ParserTest, Math) {
  std::string content = R"(some $$1+2*3$$ math)";
  DoParserTest(content, ParseTreeComparer({{ParseTreeNode::NODE, 0, 19, 0},
                                           {ParseTreeNode::PARAGRAPH, 0, 19, 1},
                                           {ParseTreeNode::MATH, 5, 14, 2}}));
}

TEST(ParserTest, Quote) {
  std::string content = R"(
> some quote
> continued
not quote
)";
  DoParserTest(content, ParseTreeComparer({
                            {ParseTreeNode::NODE, 0, 36, 0},
                            {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                            {ParseTreeNode::QUOTE, 1, 26, 1},
                            {ParseTreeNode::TEXT, 3, 13, 2},
                            {ParseTreeNode::TEXT, 16, 25, 2},
                            {ParseTreeNode::PARAGRAPH, 26, 36, 1},
                        }));
}

TEST(ParserTest, NotQuote) {
  std::string content = R"(
  > this is not quote
)";
  DoParserTest(content,
               ParseTreeComparer({{ParseTreeNode::NODE, 0, 23, 0},
                                  {ParseTreeNode::PARAGRAPH, 0, 23, 1}}));
}

TEST(ParserTest, SideNote) {
  std::string content = R"(
```sidenote
1. a
2. b
```
)";

  // TODO Fix start and end inversion issue on Paragraph node (23, 22)
  DoParserTest(content, ParseTreeComparer({
                            {ParseTreeNode::NODE, 0, 27, 0},
                            {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                            {ParseTreeNode::BOX, 1, 26, 1},
                            {ParseTreeNode::TEXT, 4, 12, 2},
                            {ParseTreeNode::NODE, 13, 26, 2},
                            {ParseTreeNode::ORDERED_LIST, 13, 23, 3},
                            {ParseTreeNode::ORDERED_LIST_ITEM, 13, 18, 4},
                            {ParseTreeNode::PARAGRAPH, 16, 17, 5},
                            {ParseTreeNode::ORDERED_LIST_ITEM, 18, 23, 4},
                            {ParseTreeNode::PARAGRAPH, 21, 22, 5},
                            {ParseTreeNode::PARAGRAPH, 23, 22, 3},
                            {ParseTreeNode::PARAGRAPH, 26, 27, 1},
                        }));
}

TEST(ParserTest, SimpleRef) {
  std::string content = R"(
```ref-abc
some stuff
```

this is \ref{abc}.
)";

  DoParserTest(content, ParseTreeComparer({
                            {ParseTreeNode::NODE, 0, 47, 0},
                            {ParseTreeNode::PARAGRAPH, 0, 1, 1},
                            {ParseTreeNode::BOX, 1, 26, 1},
                            {ParseTreeNode::TEXT, 4, 11, 2},
                            {ParseTreeNode::NODE, 12, 26, 2},
                            {ParseTreeNode::TEXT, 12, 22, 3},
                            {ParseTreeNode::PARAGRAPH, 26, 26, 1},
                            {ParseTreeNode::PARAGRAPH, 28, 47, 1},
                            {ParseTreeNode::COMMAND, 36, 45, 2},
                            {ParseTreeNode::TEXT, 41, 44, 3},
                        }));
}

}  // namespace
}  // namespace md2

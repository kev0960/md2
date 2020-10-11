#include "../src/parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "parse_tree_builder.h"

namespace md2 {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

void DoParserTest(std::string content, ParseTreeComparer comparer) {
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);
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

}  // namespace
}  // namespace md2

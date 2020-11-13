
#include "../src/generators/latex_generator.h"
#include "../src/parser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace md2 {
namespace {

using ::testing::Eq;

void DoLatexTest(std::string content, std::string expected) {
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  MetadataRepo repo;
  GeneratorContext context(repo, "image_path");
  LatexGenerator generator(/*filename=*/"some_file.md", content, context);
  generator.Generate(tree);

  EXPECT_EQ(std::string(generator.ShowOutput()), expected);
}

TEST(LatexTest, Paragraph) { DoLatexTest("a", "\na\n"); }
TEST(LatexTest, ParagraphLongerLines) { DoLatexTest("abc", "\nabc\n"); }
TEST(LatexTest, TwoParagraphs) { DoLatexTest("a\n\nb", "\na\n\nb\n"); }
TEST(LatexTest, BoldInParagraph) {
  DoLatexTest("a**b**c", "\na\\textbf{b}c\n");
}

TEST(LatexTest, ItalicInParagraph) { DoLatexTest("a*b*c", "\na\\emph{b}c\n"); }

TEST(LatexTest, ItalicInBold) {
  DoLatexTest("***a***", "\n\\textbf{\\emph{a}}\n");
}

TEST(LatexTest, ItalicInBold2) {
  // a<b>b<i>c</i><i>d</i></b>
  DoLatexTest("a**b*c**d***", "\na\\textbf{b\\emph{c}\\emph{d}}\n");
}

TEST(LatexTest, ItalicInBold3) {
  // a<b>b<i>c</i><i>d</i></b>
  DoLatexTest("a**b*c**d***", "\na\\textbf{b\\emph{c}\\emph{d}}\n");
}

TEST(LatexTest, ItalicInBold4) {
  // a<b><i>b</i><i>c</i></b><b>a</b>
  DoLatexTest("a***b**c*****a**",
              "\na\\textbf{\\emph{b}\\emph{c}}\\textbf{a}\n");
}

TEST(LatexTest, BoldInItalic) {
  // <i>a<b>b</b></i>
  DoLatexTest("*a**b***", "\n\\emph{a\\textbf{b}}\n");
}

TEST(LatexTest, BoldInItalic2) {
  // <i>a<b>b</b><b>c</b></i><b>e</b>
  DoLatexTest("*a**b****c***", "\n\\emph{a\\textbf{b}\\textbf{c}}\n");
}

TEST(LatexTest, StrikeThrough) { DoLatexTest("~~ab~~", "\n\\sout{ab}\n"); }

TEST(LatexTest, StrikeThrough2) {
  DoLatexTest("a~~a*b*~~c", "\na\\sout{a\\emph{b}}c\n");
}

}  // namespace
}  // namespace md2

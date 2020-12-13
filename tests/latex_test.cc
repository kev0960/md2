
#include "../src/generators/latex_generator.h"
#include "../src/parser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "logger.h"

namespace md2 {
namespace {

using ::testing::Eq;

void DoLatexTest(std::string content, std::string expected) {
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  MetadataRepo repo;
  GeneratorContext context(repo, "image_path");
  LatexGenerator generator(/*filename=*/"some_file.md", content, context, tree);
  generator.Generate();

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

TEST(LatexTest, Link) {
  DoLatexTest("[link](http://link)", "\n\\href{http://link}{link}\n");
}

TEST(LatexTest, Link2) {
  // "]" inside of the ** should be ignored.
  DoLatexTest("[link *]*](http://link)",
              "\n\\href{http://link}{link \\emph{]}}\n");
}

TEST(LatexTest, Link3) {
  DoLatexTest("arr[0][1][2](link)", "\narr[0][1]\\href{link}{2}\n");
}

TEST(LatexTest, InvalidLink) {
  DoLatexTest("[link] (http://link)", "\n[link] (http://link)\n");
}

TEST(LatexTest, InvalidLink2) {
  DoLatexTest("arr[0][1][2]", "\narr[0][1][2]\n");
}

TEST(LatexTest, Image) {
  DoLatexTest("![alttext](http://img)",
              "\n\n\\begin{figure}[H]\n\\centering\n\\includegraphics[max "
              "width=0.7\\linewidth]{http://img}\n\\end{figure}\n\n");
}

TEST(LatexTest, ImageWithCaption) {
  DoLatexTest(
      "![alttext caption=abc](http://img)",
      "\n\n\\begin{figure}[H]\n\\centering\n\\includegraphics[max "
      "width=0.7\\linewidth]{http://img}\n\\caption*{abc}\n\\end{figure}\n\n");
}

TEST(LatexTest, ImageWithCaptionBold) {
  DoLatexTest("![alttext caption=some**aa**](http://img)",
              "\n\n\\begin{figure}[H]\n\\centering\n\\includegraphics[max "
              "width=0.7\\linewidth]{http://"
              "img}\n\\caption*{some\\textbf{aa}}\n\\end{figure}\n\n");
}

TEST(LatexTest, ImageWithCaptionComplex) {
  // "caption=" inside of ** should be ignored.
  DoLatexTest(
      "![alttext caption=some**cap***a*](http://img)",
      "\n\n\\begin{figure}[H]\n\\centering\n\\includegraphics[max "
      "width=0.7\\linewidth]{http://"
      "img}\n\\caption*{some\\textbf{cap}\\emph{a}}\n\\end{figure}\n\n");
}

TEST(LatexTest, SimpleTable) {
  std::string content = R"(
|a|b|c|
|-|-|-|
|a|b|c|
abc)";

  DoLatexTest(content,
              "\n\n\n\n\\begin{tabularx}{\\textwidth}{|X|X|X|}\n\\hline\n\na\n "
              "& \nb\n & \nc\n \\\\ \\hline\n \na\n & \nb\n & \nc\n \\\\ "
              "\\hline\n \\end{tabularx}\nabc\n");
}

TEST(LatexTest, SimpleTable2) {
  std::string content = R"(
|*a*|**b**|
|-|-|
|\|a|`b`|)";

  DoLatexTest(
      content,
      "\n\n\n\n\\begin{tabularx}{\\textwidth}{|X|X|}"
      "\n\\hline\n\n\\emph{a}\n & \n\\textbf{b}\n \\\\ \\hline\n \n|a\n "
      "& \n\\texttt{b}\n \\\\ \\hline\n \\end{tabularx}");
}

TEST(LatexTest, SimpleVerbatim) {
  DoLatexTest("a `code` a", "\na \\texttt{code} a\n");
}

TEST(LatexTest, SimpleList) {
  std::string content = R"(
* a
* b
  c
* d)";

  DoLatexTest(content,
              "\n\n\n\n\\begin{itemize}\n\\item \na\n\n\\item \nb\n\n  "
              "c\n\n\\item \nd\n\n\\end{itemize}");
}

TEST(LatexTest, SimpleNestedList) {
  std::string content = R"(
* a
  * b
    * c
    * d
* e)";

  DoLatexTest(content,
              "\n\n\n\n\\begin{itemize}\n\\item "
              "\na\n\n\\begin{itemize}\n\\item \nb\n\n\\begin{itemize}\n\\item "
              "\nc\n\n\\item \nd\n\n\\end{itemize}\n\\end{itemize}\n\\item "
              "\ne\n\n\\end{itemize}");
}

TEST(LatexTest, SimpleNestedListWithSomeText) {
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

  DoLatexTest(content,
              "\n\n\n\n\\begin{itemize}\n\\item \na\n\n  "
              "a2\n\n\\begin{itemize}\n\\item \nb1\n\n\\begin{itemize}\n\\item "
              "\nc1\n\n\\item \nd1\n\n\\end{itemize}\n\\item \nb2\n\n    "
              "b3\n\n\\begin{itemize}\n\\item "
              "\nc2\n\n\\end{itemize}\n\\end{itemize}\n\\item "
              "\ne\n\n\\end{itemize}");
}

TEST(LatexTest, ParagraphMiddleOfList) {
  std::string content = R"(
* a

some text
* e)";

  DoLatexTest(content,
              "\n\n\n\n\\begin{itemize}\n\\item \na\n\n\\end{itemize}\nsome "
              "text\n\n\n\\begin{itemize}\n\\item \ne\n\n\\end{itemize}");
}

TEST(LatexTest, ParagraphMiddleOfListWithLongEmptyNewline) {
  // Note the empty line above some text is "   \n", not just "\n".
  std::string content = R"(
* a

some text
* e)";

  DoLatexTest(content,
              "\n\n\n\n\\begin{itemize}\n\\item \na\n\n\\end{itemize}\nsome "
              "text\n\n\n\\begin{itemize}\n\\item \ne\n\n\\end{itemize}");
}

TEST(LatexTest, HeaderSimple) {
  DoLatexTest("### header", "\n\\subsection{ header}\n");
}

TEST(LatexTest, HeaderSimple2) {
  DoLatexTest("a\n### header", "\na\n\n\n\\subsection{ header}\n");
}

TEST(LatexTest, NotHeader) {
  DoLatexTest("a### header", "\na\\#\\#\\# header\n");
}

TEST(LatexTest, SimpleCommand) {
  std::string content = R"(\sidenote{this *is* a sidenote})";
  DoLatexTest(content, "\n\\footnote{this \\emph{is} a sidenote}\n");
}

TEST(LatexTest, NestedBox) {
  std::string content = R"(
```info-text
hello
```sidenote
*a*
```
b
```)";

  DoLatexTest(content,
              "\n\n\n\n\\begin{tcolorbox}[colback=green!5!white,colframe=green!"
              "75!black,left=3pt,right=3pt,enlarge top "
              "by=2mm]\n\nhello\n\n\\footnote{\n\\emph{a}\n}\n\nb\n\n\\end{"
              "tcolorbox}\n");
}

TEST(LatexTest, SideNote) {
  std::string content = R"(
```sidenote
1. a
2. b
```
)";
  DoLatexTest(content,
              "\n\n\n\\footnote{\n\\begin{enumerate}\n\\item \na\n\n\\item "
              "\nb\n\n\\end{enumerate}}\n\n\n");
}

TEST(LatexTest, Quote) {
  std::string content = R"(
> some quote
> continued
not quote
)";
  DoLatexTest(content,
              "\n\n\n\n\\begin{displayquote}\nsome "
              "quotecontinued\n\\end{displayquote}\n\nnot quote\n\n");
}

TEST(LatexTest, NotQuote) {
  std::string content = R"(
  > this is not quote
)";
  DoLatexTest(content, "\n\n  > this is not quote\n\n");
}

}  // namespace
}  // namespace md2

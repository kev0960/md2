#include "../src/generators/html_generator.h"
#include "../src/parser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace md2 {
namespace {

using ::testing::Eq;

void DoHtmlTest(std::string content, std::string expected) {
  Parser parser;
  ParseTree tree = parser.GenerateParseTree(content);

  MetadataRepo repo;
  GeneratorContext context(repo, "image_path");
  HTMLGenerator generator(/*filename=*/"some_file.md", content, context, tree);
  generator.Generate();

  EXPECT_EQ(std::string(generator.ShowOutput()), expected);
}

TEST(HtmlTest, Paragraph) { DoHtmlTest("a", "<p>a</p>"); }
TEST(HtmlTest, ParagraphLongerLines) { DoHtmlTest("abc", "<p>abc</p>"); }
TEST(HtmlTest, TwoParagraphs) { DoHtmlTest("a\n\nb", "<p>a</p><p>b</p>"); }
TEST(HtmlTest, BoldInParagraph) {
  DoHtmlTest("a**b**c", "<p>a<span class='font-weight-bold'>b</span>c</p>");
}

TEST(HtmlTest, ItalicInParagraph) {
  DoHtmlTest("a*b*c", "<p>a<span class='font-italic'>b</span>c</p>");
}

TEST(HtmlTest, ItalicInBold) {
  DoHtmlTest("***a***",
             "<p><span class='font-weight-bold'><span "
             "class='font-italic'>a</span></span></p>");
}

TEST(HtmlTest, ItalicInBold2) {
  // a<b>b<i>c</i><i>d</i></b>
  DoHtmlTest("a**b*c**d***",
             "<p>a<span class='font-weight-bold'>b<span "
             "class='font-italic'>c</span><span "
             "class='font-italic'>d</span></span></p>");
}

TEST(HtmlTest, ItalicInBold3) {
  // a<b>b<i>c</i><i>d</i></b>
  DoHtmlTest("a**b*c**d***",
             "<p>a<span class='font-weight-bold'>b<span "
             "class='font-italic'>c</span><span "
             "class='font-italic'>d</span></span></p>");
}

TEST(HtmlTest, ItalicInBold4) {
  // a<b><i>b</i><i>c</i></b><b>a</b>
  DoHtmlTest("a***b**c*****a**",
             "<p>a<span class='font-weight-bold'><span "
             "class='font-italic'>b</span><span "
             "class='font-italic'>c</span></span><span "
             "class='font-weight-bold'>a</span></p>");
}

TEST(HtmlTest, BoldInItalic) {
  // <i>a<b>b</b></i>
  DoHtmlTest("*a**b***",
             "<p><span class='font-italic'>a<span "
             "class='font-weight-bold'>b</span></span></p>");
}

TEST(HtmlTest, BoldInItalic2) {
  // <i>a<b>b</b><b>c</b></i><b>e</b>
  DoHtmlTest("*a**b****c***",
             "<p><span class='font-italic'>a<span "
             "class='font-weight-bold'>b</span><span "
             "class='font-weight-bold'>c</span></span></p>");
}

TEST(HtmlTest, StrikeThrough) {
  DoHtmlTest("~~ab~~", "<p><span class='font-strike'>ab</span></p>");
}

TEST(HtmlTest, StrikeThrough2) {
  DoHtmlTest("a~~a*b*~~c",
             "<p>a<span class='font-strike'>a<span "
             "class='font-italic'>b</span></span>c</p>");
}

TEST(HtmlTest, Link) {
  DoHtmlTest("[link](http://link)", "<p><a href='http://link'>link</a></p>");
}

TEST(HtmlTest, Link2) {
  // "]" inside of the ** should be ignored.
  DoHtmlTest("[link *]*](http://link)",
             "<p><a href='http://link'>link <span "
             "class='font-italic'>]</span></a></p>");
}

TEST(HtmlTest, Link3) {
  DoHtmlTest("arr[0][1][2](link)", "<p>arr[0][1]<a href='link'>2</a></p>");
}

TEST(HtmlTest, InvalidLink) {
  DoHtmlTest("[link] (http://link)", "<p>[link] (http://link)</p>");
}

TEST(HtmlTest, InvalidLink2) {
  DoHtmlTest("arr[0][1][2]", "<p>arr[0][1][2]</p>");
}

TEST(HtmlTest, EscapeChar) {
  // \b and \note are not escaped.
  DoHtmlTest(R"(\*\b\\\note)", R"(<p>*\b\\note</p>)");
}

TEST(HtmlTest, Image) {
  DoHtmlTest("![alttext](http://img)",
             "<p><figure><picture><img class='content-img' src='http://img' "
             "alt='alttext'></picture><figcaption></figcaption></figure></p>");
}

TEST(HtmlTest, ImageWithCaption) {
  DoHtmlTest(
      "![alttext caption=abc](http://img)",
      "<p><figure><picture><img class='content-img' src='http://img' "
      "alt='alttext '></picture><figcaption>abc</figcaption></figure></p>");
}

TEST(HtmlTest, ImageWithCaptionBold) {
  DoHtmlTest("![alttext caption=some**aa**](http://img)",
             "<p><figure><picture><img class='content-img' src='http://img' "
             "alt='alttext '></picture><figcaption>some<span "
             "class='font-weight-bold'>aa</span></figcaption></figure></p>");
}

TEST(HtmlTest, ImageWithCaptionComplex) {
  // "caption=" inside of ** should be ignored.
  DoHtmlTest("![alttext caption=some**cap***a*](http://img)",
             "<p><figure><picture><img class='content-img' src='http://img' "
             "alt='alttext '></picture><figcaption>some<span "
             "class='font-weight-bold'>cap</span><span "
             "class='font-italic'>a</span></figcaption></figure></p>");
}

TEST(HtmlTest, ImageWithCaptionAndSize) {
  // "caption=" inside of ** should be ignored.
  DoHtmlTest(
      "![alttext caption=cap size=123x123](http://img)",
      "<p><figure><picture><img class='content-img' src='http://img' "
      "alt='alttext '></picture><figcaption>cap </figcaption></figure></p>");
}

TEST(HtmlTest, ImageWithCaptionItalicAndSize) {
  // "caption=" inside of ** should be ignored.
  DoHtmlTest("![alttext caption=*a* size=123x123](http://img)",
             "<p><figure><picture><img class='content-img' src='http://img' "
             "alt='alttext '></picture><figcaption><span "
             "class='font-italic'>a</span> </figcaption></figure></p>");
}

TEST(HtmlTest, ImageWithCaptionItalicAndSizeNoAlt) {
  // "caption=" inside of ** should be ignored.
  DoHtmlTest("![caption=*a* size=123x123](http://img)",
             "<p><figure><picture><img class='content-img' src='http://img' "
             "alt=''></picture><figcaption><span class='font-italic'>a</span> "
             "</figcaption></figure></p>");
}

TEST(HtmlTest, HeaderSimple) {
  DoHtmlTest("### header",
             "<h3 id='page-heading-0' class='header-general'> header</h3>");
}

TEST(HtmlTest, HeaderSimple2) {
  DoHtmlTest("a\n### header",
             "<p>a\n</p><h3 id='page-heading-0' class='header-general'> "
             "header</h3>");
}

TEST(HtmlTest, LectureHeader) {
  DoHtmlTest("###@ [Some](lecture)",
             "<h3 class='lecture-header' id='page-heading-0' "
             "class='header-general'> <a href='lecture'>Some</a></h3>");
}

TEST(HtmlTest, NotHeader) { DoHtmlTest("a### header", "<p>a### header</p>"); }

TEST(HtmlTest, SimpleVerbatim) {
  DoHtmlTest("a `code` a", "<p>a <code class='inline-code'>code</code> a</p>");
}

TEST(HtmlTest, SimpleCode) {
  DoHtmlTest(R"(
```cpp
hello;
```)",
             "<p>\n</p><pre class='chroma lang-cpp'><span "
             "class='i'>hello;\n</span></pre>");
}

TEST(HtmlTest, NestedBox) {
  std::string content = R"(
```note
hello
```note
*a*
```
b
```)";

  DoHtmlTest(content,
             "<p>\n</p><div class='inline-note'><p>hello\n</p><div "
             "class='inline-note'><p><span "
             "class='font-italic'>a</span></p></div><p>\nb</p></div>");
}

TEST(HtmlTest, SimpleTable) {
  std::string content = R"(
|a|b|c|
|-|-|-|
|a|b|c|
abc)";

  DoHtmlTest(
      content,
      "<p>\n</p><table><thead><tr><th><p>a</p></th><th><p>b</p></th><th><p>c</"
      "p></th></tr></thead><tbody><tr><td><p>a</p></td><td><p>b</p></"
      "td><td><p>c</p></td></tr></tbody></table><p>abc</p>");
}

TEST(HtmlTest, SimpleTable2) {
  std::string content = R"(
|*a*|**b**|
|-|-|
|\|a|`b`|)";

  DoHtmlTest(content,
             "<p>\n</p><table><thead><tr><th><p><span "
             "class='font-italic'>a</span></p></th><th><p><span "
             "class='font-weight-bold'>b</span></p></th></tr></"
             "thead><tbody><tr><td><p>|a</p></td><td><p><code "
             "class='inline-code'>b</code></p></td></tr></tbody></table>");
}

TEST(HtmlTest, SimpleList) {
  std::string content = R"(
* a
* b
  c
* d)";

  DoHtmlTest(content,
             "<p>\n</p><ul><li><p>a</p></li><li><p>b</p><p>  "
             "c</p></li><li><p>d</p></li></ul>");
}

TEST(HtmlTest, SimpleNestedList) {
  std::string content = R"(
* a
  * b
    * c
    * d
* e)";

  DoHtmlTest(content,
             "<p>\n</p><ul><li><p>a</p></li><ul><li><p>b</p></li><ul><li><p>c</"
             "p></li><li><p>d</p></li></ul></ul><li><p>e</p></li></ul>");
}

TEST(HtmlTest, SimpleNestedListWithSomeText) {
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

  DoHtmlTest(content,
             "<p>\n</p><ul><li><p>a</p><p>  "
             "a2</p></li><ul><li><p>b1</p></li><ul><li><p>c1</p></"
             "li><li><p>d1</p></li></ul><li><p>b2</p><p>    "
             "b3</p></li><ul><li><p>c2</p></li></ul></ul><li><p>e</p></li></"
             "ul>");
}

TEST(HtmlTest, ParagraphMiddleOfList) {
  std::string content = R"(
* a

some text
* e)";

  DoHtmlTest(content,
             "<p>\n</p><ul><li><p>a</p></li></ul><p>some "
             "text\n</p><ul><li><p>e</p></li></ul>");
}

TEST(HtmlTest, ParagraphMiddleOfListWithLongEmptyNewline) {
  // Note the empty line above some text is "   \n", not just "\n".
  std::string content = R"(
* a

some text
* e)";

  DoHtmlTest(content,
             "<p>\n</p><ul><li><p>a</p></li></ul><p>some "
             "text\n</p><ul><li><p>e</p></li></ul>");
}

/*
TEST(HtmlTest, SimpleListWithVerbatim) {
  std::string content = R"(
* code
```cpp
abc
```
* code2
```cpp
def
```)";

  DoHtmlTest(content, "");
}
*/

TEST(HtmlTest, SimpleOrderedList) {
  std::string content = R"(
1. a
2. b
  c
3. d)";

  DoHtmlTest(content,
             "<p>\n</p><ol><li><p>a</p></li><li><p>b</p><p>  "
             "c</p></li><li><p>d</p></li></ol>");
}

TEST(HtmlTest, OrderedAndUnorderedMixed) {
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

  DoHtmlTest(content,
             "<p>\n</p><ol><li><p>a</p></li><li><p>b</p></li><ul><li><p>c</p></"
             "li><li><p>d</p></li><ol><li><p>e</p></li><li><p>f</p></li></"
             "ol><li><p>g</p></li><ul><li><p>a</p></li></ul></ul><li><p>d</p></"
             "li></ol>");
}

TEST(HtmlTest, SimpleCommand) {
  std::string content = R"(\sidenote{this *is* a sidenote})";
  DoHtmlTest(content,
             "<p><aside class='sidenote'>this <span "
             "class='font-italic'>is</span> a sidenote</aside></p>");
}

TEST(HtmlTest, MultipleArgCommand) {
  std::string content = R"(\tooltip{**a**}{**})";
  DoHtmlTest(content,
             "<p><span class='page-tooltip' data-tooltip='**' "
             "data-tooltip-position='bottom'><span "
             "class='font-weight-bold'>a</span></span></p>");
}

TEST(HtmlTest, CommandAndInvalidCommand) {
  std::string content = R"(\tooltip{a}{b} \tooltip{a})";
  DoHtmlTest(content,
             "<p><span class='page-tooltip' data-tooltip='b' "
             "data-tooltip-position='bottom'>a</span> \\tooltip{a}</p>");
}

TEST(HtmlTest, Math) {
  std::string content = R"(some $$1+2*3$$ math)";
  DoHtmlTest(content,
             "<p>some <span class='math-latex'>$1+2*3$</span> math</p>");
}

TEST(HtmlTest, SideNote) {
  std::string content = R"(
```sidenote
1. a
2. b
```
)";
  DoHtmlTest(content,
             "<p>\n</p><aside "
             "class='sidenote'><ol><li><p>a</p></li><li><p>b</p></li></ol>"
             "</aside><p>\n</p>");
}

TEST(HtmlTest, Quote) {
  std::string content = R"(
> some quote
> continued
not quote
)";
  DoHtmlTest(content,
             "<p>\n</p><blockquote class='quote'>some "
             "quotecontinued</blockquote><p>not quote\n</p>");
}

TEST(HtmlTest, NotQuote) {
  std::string content = R"(
  > this is not quote
)";
  DoHtmlTest(content, "<p>\n  &gt; this is not quote\n</p>");
}

TEST(HtmlTest, SimpleRef) {
  std::string content = R"(
```ref-abc
some stuff
```

this is \ref{abc}.
)";

  DoHtmlTest(content, "<p>\n</p><p>this is some stuff.\n</p>");
}

TEST(HtmlTest, TableWithRef) {
  std::string content = R"(
|a|b|
|-|-|
|\ref{code1}|\ref{code2}|

```ref-code1
```cpp
some code1
```
```

```ref-code2
```cpp
some code2
```
```)";

  DoHtmlTest(content,
             "<p>\n</p><table><thead><tr><th><p>a</p></th><th><p>b</p></th></"
             "tr></thead><tbody><tr><td><p><pre class='chroma lang-cpp'><span "
             "class='i'>some code1\n</span></pre></p></td><td><p><pre "
             "class='chroma lang-cpp'><span class='i'>some "
             "code2\n</span></pre></p></td></tr></tbody></table><p>\n</p>");
}

}  // namespace
}  // namespace md2

#include "html_generator.h"

#include <fmt/core.h>

#include "asm_syntax_highlighter.h"
#include "cpp_syntax_highlighter.h"
#include "generator_util.h"
#include "logger.h"
#include "objdump_highlighter.h"
#include "py_syntax_highlighter.h"

namespace md2 {
namespace {

constexpr std::string_view kChewingCEnd = R"(
<div class='next-lecture-box'>강좌를 보다가 조금이라도 <span class='font-weight-bold'>궁금한 것이나 이상한 점이 있다면 꼭 댓글</span>을 남겨주시기 바랍니다. 그 외에도 강좌에 관련된 것이라면 어떠한 것도 질문해 주셔도 상관 없습니다. 생각해 볼 문제도 정 모르겠다면 댓글을 달아주세요. <br><br>
현재 여러분이 보신 강좌는 <span class='font-italic lecture-title'>&lt;{}&gt;</span> 입니다. 이번 강좌의 모든 예제들의 코드를 보지 않고 짤 수준까지 강좌를 읽어 보시기 전까지 다음 강좌로 넘어가지 말아주세요
<div class="next-lecture"><a href="/notice/15">다음 강좌 보러가기</a></div></div>
)";

constexpr std::string_view kChewingCppEnd = R"(
<div class='next-lecture-box'>강좌를 보다가 조금이라도 <span class='font-weight-bold'>궁금한 것이나 이상한 점이 있다면 꼭 댓글</span>을 남겨주시기 바랍니다. 그 외에도 강좌에 관련된 것이라면 어떠한 것도 질문해 주셔도 상관 없습니다. 생각해 볼 문제도 정 모르겠다면 댓글을 달아주세요. <br><br>
현재 여러분이 보신 강좌는 <span class='font-italic lecture-title'>&lt;{}&gt;</span> 입니다. 이번 강좌의 모든 예제들의 코드를 보지 않고 짤 수준까지 강좌를 읽어 보시기 전까지 다음 강좌로 넘어가지 말아주세요
<div class="next-lecture"><a href="/135">다음 강좌 보러가기</a></div></div>
)";

constexpr std::string_view kCppRefStart = R"(
<div class='cpp-ref-start'><p class='cpp-ref-link'>이 레퍼런스의 모든 내용은 <a href="https://cppreference.com">여기</a>를 기초로 하여 작성하였습니다.</p><p class='cpp-lec-introduce'>아직 C++ 에 친숙하지 않다면 <a href="https://modoocode.com/135">씹어먹는 C++</a> 은 어때요?</p></div>
)";

std::string RunSyntaxHighlighter(const GeneratorContext& context,
                                 std::string_view code,
                                 std::string_view language) {
  std::string str_code(code);
  std::string str_lang(language);

  std::unique_ptr<SyntaxHighlighter> highlighter;
  if (language == "cpp") {
    highlighter = std::make_unique<CppSyntaxHighlighter>(str_code, str_lang);
  } else if (language == "py") {
    highlighter = std::make_unique<PySyntaxHighlighter>(str_code, str_lang);
  } else if (language == "asm") {
    highlighter =
        std::make_unique<AsmSyntaxHighlighter>(context, str_code, str_lang);
  } else if (language == "objdump") {
    highlighter =
        std::make_unique<ObjdumpHighlighter>(context, str_code, str_lang);
  } else {
    return str_code;
  }

  highlighter->ParseCode();
  highlighter->ColorMerge();
  return highlighter->GenerateHighlightedHTML();
}

// Returns "" if it does not need to be escaped.
std::string_view EscapeHtmlChar(char c) {
  if (c == '<') {
    return "&lt;";
  } else if (c == '>') {
    return "&gt;";
  } else if (c == '&') {
    return "&amp;";
  }

  return "";
}

std::string EscapeString(std::string_view s) {
  std::string escaped;
  escaped.reserve(s.size());
  for (char c : s) {
    if (std::string_view esc = EscapeHtmlChar(c); !esc.empty()) {
      escaped.append(esc);
    } else {
      escaped.push_back(c);
    }
  }

  return escaped;
}

}  // namespace

void HTMLGenerator::HandleParseTreeNode(const ParseTreeNode& node) {
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

void HTMLGenerator::EmitChar(int index) {
  if (should_escape_html_) {
    std::string_view escaped = EscapeHtmlChar(md_[index]);
    if (!escaped.empty()) {
      GetCurrentTarget()->append(escaped);
    } else
      GetCurrentTarget()->push_back(md_[index]);
  } else {
    GetCurrentTarget()->push_back(md_[index]);
  }
}

void HTMLGenerator::EmitChar(int from, int to) {
  for (int i = from; i < to; i++) {
    EmitChar(i);
  }
}

void HTMLGenerator::HandleParagraph(const ParseTreeParagraphNode& node) {
  // Do not emit the empty or mal-formed paragraph.
  if (node.Start() >= node.End()) {
    return;
  }

  GetCurrentTarget()->append("<p>");

  GenerateWithDefaultAction(node, [this](int index) { EmitChar(index); });

  GetCurrentTarget()->append("</p>");
}

void HTMLGenerator::HandleText(const ParseTreeTextNode& node) {
  GenerateWithDefaultAction(node, [this](int index) { EmitChar(index); });
}

void HTMLGenerator::HandleBold(const ParseTreeBoldNode& node) {
  GetCurrentTarget()->append("<span class='font-weight-bold'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 2,
      node.End() - 2);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleItalic(const ParseTreeItalicNode& node) {
  GetCurrentTarget()->append("<span class='font-italic'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 1,
      node.End() - 1);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleStrikeThrough(
    const ParseTreeStrikeThroughNode& node) {
  GetCurrentTarget()->append("<span class='font-strike'>");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 2,
      node.End() - 2);

  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleLink(const ParseTreeLinkNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "Number of children is not two");

  links_.push_back(HTMLLinkBuilder());

  targets_.push_back(&links_.back().link_desc);
  HandleParseTreeNode(*node.GetChildren()[0]);
  targets_.pop_back();

  targets_.push_back(&links_.back().link_url);
  HandleParseTreeNode(*node.GetChildren()[1]);
  targets_.pop_back();

  const HTMLLinkBuilder& link = links_.back();

  std::pair<std::string_view, std::string_view> ref_link_and_name =
      context_->FindReference(link.link_desc);

  auto [ref_link, ref_name] = ref_link_and_name;
  if (!ref_link.empty()) {
    // We should emit the link to the inline code instead, but use the link
    // specified in the link (not the reference link).
    GetCurrentTarget()->append(fmt::format("<a href='{}' class='link-code'>",
                                           StripItguruLink(link.link_url)));
    GetCurrentTarget()->append(link.link_desc);
    GetCurrentTarget()->append("</a>");
  } else {
    GetCurrentTarget()->append(StrCat("<a href='",
                                      StripItguruLink(link.link_url), "'>",
                                      link.link_desc, "</a>"));
  }

  links_.pop_back();
}

void HTMLGenerator::HandleImage(const ParseTreeImageNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "Number of children is not two");

  const ParseTreeNode* desc_node = node.GetChildren()[0].get();
  MD2_ASSERT(desc_node->GetNodeType() == ParseTreeNode::NODE, "");

  const ParseTreeNode* desc = desc_node->GetChildren()[0].get();
  MD2_ASSERT(desc->GetNodeType() == ParseTreeNode::TEXT, "");

  images_.push_back(HTMLImageBuilder());

  const auto keyword_to_index = node.GetKeywordToIndex();
  if (auto index = keyword_to_index.find("alt");
      index != keyword_to_index.end()) {
    targets_.push_back(&images_.back().alt);
    HandleParseTreeNode(*desc->GetChildren().at(index->second));
    targets_.pop_back();
  }

  if (auto index = keyword_to_index.find("caption");
      index != keyword_to_index.end()) {
    targets_.push_back(&images_.back().caption);
    HandleParseTreeNode(*desc->GetChildren().at(index->second));
    targets_.pop_back();
  }

  if (auto index = keyword_to_index.find("size");
      index != keyword_to_index.end()) {
    targets_.push_back(&images_.back().size);
    HandleParseTreeNode(*desc->GetChildren().at(index->second));
    targets_.pop_back();
  }

  targets_.push_back(&images_.back().url);
  HandleParseTreeNode(*node.GetChildren()[1]);
  targets_.pop_back();

  const HTMLImageBuilder& image = images_.back();
  GetCurrentTarget()->append(StrCat(
      "<figure><picture><img class='content-img' src='",
      context_->FindImageForHtml(image.url), "' alt='", image.alt, "'>",
      "</picture><figcaption>", image.caption, "</figcaption></figure>"));
  images_.pop_back();
}

void HTMLGenerator::HandleTable(const ParseTreeTableNode& node) {
  GetCurrentTarget()->append("<table><thead><tr>");

  const size_t row_size = node.GetRowSize();

  // The first row is always the header.
  for (size_t i = 0; i < row_size; i++) {
    GetCurrentTarget()->append("<th>");
    HandleParseTreeNode(*node.GetChildren()[i]);
    GetCurrentTarget()->append("</th>");
  }

  GetCurrentTarget()->append("</tr></thead><tbody>");

  // The second row of the table always indicates the alignment of the cells.
  // TODO Set the alignmnent of the columns.

  for (size_t i = 2 * row_size; i < node.GetChildren().size(); i++) {
    if (i % row_size == 0) {
      GetCurrentTarget()->append("<tr>");
    }

    GetCurrentTarget()->append("<td>");
    HandleParseTreeNode(*node.GetChildren()[i]);
    GetCurrentTarget()->append("</td>");

    if (i % row_size == row_size - 1) {
      GetCurrentTarget()->append("</tr>");
    }
  }

  GetCurrentTarget()->append("</tbody></table>");
}

void HTMLGenerator::HandleList(const ParseTreeListNode& node) {
  if (node.IsOrdered()) {
    GetCurrentTarget()->append("<ol>");
  } else {
    GetCurrentTarget()->append("<ul>");
  }

  for (const auto& child : node.GetChildren()) {
    HandleParseTreeNode(*child);
  }

  if (node.IsOrdered()) {
    GetCurrentTarget()->append("</ol>");
  } else {
    GetCurrentTarget()->append("</ul>");
  }
}

void HTMLGenerator::HandleListItem(const ParseTreeListItemNode& node) {
  GetCurrentTarget()->append("<li>");

  for (const auto& child : node.GetChildren()) {
    HandleParseTreeNode(*child);
  }

  GetCurrentTarget()->append("</li>");
}

void HTMLGenerator::HandleHeader(const ParseTreeHeaderNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "");

  std::string_view header_symbol = GetStringInNode(node.GetChildren()[0].get());

  if (std::all_of(header_symbol.begin(), header_symbol.end(),
                  [](const char c) { return c == '#'; })) {
    std::string header_id =
        StrCat("id='page-heading-", std::to_string(header_index_++), "'");
    GetCurrentTarget()->append(StrCat("<h",
                                      std::to_string(header_symbol.size()), " ",
                                      header_id, " class='header-general'>"));
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append(
        StrCat("</h", std::to_string(header_symbol.size()), ">"));
    return;
  }

  if (header_symbol == "#@") {
    std::string header_id =
        StrCat("id='page-heading-", std::to_string(header_index_++), "'");
    GetCurrentTarget()->append(
        StrCat("<h2 class='ref-header' ", header_id, ">"));
    EmitChar(node.GetChildren()[1]->Start(), node.GetChildren()[1]->End());
    GetCurrentTarget()->append("</h2>");
  } else if (header_symbol == "##@") {
    std::string_view header_content =
        Strip(GetStringInNode(node.GetChildren()[1].get()));
    if (header_content == "chewing-c-end") {
      GetCurrentTarget()->append(fmt::format(kChewingCEnd, GetFileTitle()));
    } else if (header_content == "chewing-cpp-end") {
      GetCurrentTarget()->append(fmt::format(kChewingCppEnd, GetFileTitle()));
    } else if (header_content == "cpp-ref-start") {
      GetCurrentTarget()->append(kCppRefStart);
    }
  } else if (header_symbol == "###@") {
    std::string header_id =
        StrCat("id='page-heading-", std::to_string(header_index_++), "'");
    GetCurrentTarget()->append(StrCat("<h3 class='lecture-header' ", header_id,
                                      " class='header-general'>"));
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</h3>");
  }
}

// If the verbatim node does not have any child node, then emit the current
// text as verbatim. If it does have children, then the first node tells the
// type of code it is showing and the second node contains the actual code.
void HTMLGenerator::HandleVerbatim(const ParseTreeVerbatimNode& node) {
  if (node.GetChildren().empty()) {
    std::string_view inline_code =
        GetStringInNode(&node, /*prefix_offset=*/1, /*suffix_offset=*/1);
    std::pair<std::string_view, std::string_view> link_and_name =
        context_->FindReference(inline_code);

    auto [link, ref_name] = link_and_name;
    if (!link.empty()) {
      // We should emit the link to the inline code instead.
      GetCurrentTarget()->append(
          fmt::format("<a href='{}' class='link-code'>", link));
      GetCurrentTarget()->append(EscapeString(ref_name));
      GetCurrentTarget()->append("</a>");
    } else {
      GetCurrentTarget()->append("<code class='inline-code'>");
      GetCurrentTarget()->append(EscapeString(ref_name));
      GetCurrentTarget()->append("</code>");
    }
    return;
  }

  MD2_ASSERT(node.GetChildren().size() == 2,
             "Verbatim does not have two nodes.");
  const auto& code_name_node = node.GetChildren()[0];
  std::string_view name = md_.substr(
      code_name_node->Start(), code_name_node->End() - code_name_node->Start());

  const auto& content_node = node.GetChildren()[1];
  MD2_ASSERT(content_node->GetNodeType() == ParseTreeNode::TEXT, "");
  if (name == "cpp" || name == "info-format") {
    std::string_view formatted_cpp = context_->GetClangFormatted(
        &CastNodeTypes<ParseTreeTextNode>(*content_node), md_);
    GetCurrentTarget()->append(
        RunSyntaxHighlighter(*context_, formatted_cpp, name));
  } else if (name == "py" || name == "asm" || name == "objdump") {
    GetCurrentTarget()->append(RunSyntaxHighlighter(
        *context_, GetStringInNode(content_node.get()), name));
  } else if (name == "cpp-formatted") {
    GetCurrentTarget()->append(RunSyntaxHighlighter(
        *context_, GetStringInNode(content_node.get()), "cpp"));
  } else if (name == "compiler-warning") {
    GetCurrentTarget()->append(
        "<p class='compiler-warning-title'><i class='xi-warning'></i>컴파일 "
        "오류</p><pre class='compiler-warning'>");
    EmitChar(content_node->Start(), content_node->End());
    GetCurrentTarget()->append("</pre>");
  } else if (name == "embed") {
    bool prev_escape_state = should_escape_html_;
    should_escape_html_ = false;
    EmitChar(content_node->Start(), content_node->End());
    should_escape_html_ = prev_escape_state;
  } else if (name == "exec") {
    GetCurrentTarget()->append(
        "<p class='exec-preview-title'>실행 결과</p><pre "
        "class='exec-preview'>");
    EmitChar(content_node->Start(), content_node->End());
    GetCurrentTarget()->append("</pre>");
  } else if (name == "info" || name == "info-term" || name == "info-verb") {
    GetCurrentTarget()->append(fmt::format("<pre class='{}'>", name));
    EmitChar(content_node->Start(), content_node->End());
    GetCurrentTarget()->append("</pre>");
  }
}

void HTMLGenerator::HandleEscape(const ParseTreeEscapeNode& node) {
  EmitChar(node.Start() + 1);
}

void HTMLGenerator::HandleCommand(const ParseTreeCommandNode& node) {
  std::string_view command = node.GetCommandName();
  if (command == "sidenote") {
    GetCurrentTarget()->append("<aside class='sidenote'>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</aside>");
  } else if (command == "sc") {
    GetCurrentTarget()->append("<span class='font-smallcaps'>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</span>");
  } else if (command == "serif") {
    GetCurrentTarget()->append("<span class='font-serif-italic'>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</span>");
  } else if (command == "htmlonly") {
    HandleParseTreeNode(*node.GetChildren()[0]);
  } else if (command == "escape") {
    HandleParseTreeNode(*node.GetChildren()[0]);
  } else if (command == "footnote") {
    GetCurrentTarget()->append("<sup>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</sup>");
  } else if (command == "tooltip") {
    MD2_ASSERT(node.GetChildren().size() == 2, "");

    GetCurrentTarget()->append("<span class='page-tooltip' data-tooltip='");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("' data-tooltip-position='bottom'>");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("</span>");
  } else if (command == "newline") {
    GetCurrentTarget()->append("<br>");
  } else if (command == "ref") {
    std::string_view ref_name = GetStringInNode(node.GetChildren()[0].get());
    GetCurrentTarget()->append(
        GetReferenceNodeGeneratedOutput(std::string(ref_name)));
  }
}

void HTMLGenerator::HandleMath(const ParseTreeMathNode& node) {
  GetCurrentTarget()->append("<span class='math-latex'>");
  // Math should be enclosed by $$.
  EmitChar(node.Start() + 1, node.End() - 1);
  GetCurrentTarget()->append("</span>");
}

void HTMLGenerator::HandleBox(const ParseTreeBoxNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "");
  const auto& box_name_node = node.GetChildren()[0];

  std::string_view box_name = md_.substr(
      box_name_node->Start(), box_name_node->End() - box_name_node->Start());

  if (box_name == "info-text") {
    GetCurrentTarget()->append("<div class='info'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name == "warning") {
    GetCurrentTarget()->append("<div class='warning warning-text'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name == "lec-warning") {
    GetCurrentTarget()->append(
        "<p class='compiler-warning-title'><i class='xi-warning'></i>주의 "
        "사항</p><div class='lec-warning'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name == "lec-info") {
    GetCurrentTarget()->append(
        "<p class='lec-info-title'><i class='xi-info'></i>참고 사항</p><div "
        "class='lec-info'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name == "lec-summary") {
    GetCurrentTarget()->append(
        "<div class='lec-summary'><h3>뭘 배웠지?</h3><div "
        "class='lec-summary-content'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div></div>");
  } else if (box_name == "html-only") {
    HandleParseTreeNode(*node.GetChildren()[1]);
  } else if (box_name == "latex-only") {
    return;
  } else if (box_name == "sidenote") {
    GetCurrentTarget()->append("<aside class='sidenote'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</aside>");
  } else if (box_name == "note") {
    GetCurrentTarget()->append("<div class='inline-note'>");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("</div>");
  } else if (box_name.substr(0, 4) == "ref-") {
    return;
  }
}

void HTMLGenerator::HandleQuote(const ParseTreeQuoteNode& node) {
  GetCurrentTarget()->append("<blockquote class='quote'>");
  GenerateWithDefaultAction(node, [this](int index) {
    /* Do nothing */
    (void)(index);
    (void)(this);
  });

  GetCurrentTarget()->append("</blockquote>");
}

}  // namespace md2


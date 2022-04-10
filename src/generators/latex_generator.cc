#include "latex_generator.h"

#include <fmt/core.h>

#include "generator_util.h"

namespace md2 {
namespace {

std::string_view EscapeLatexChar(char c) {
  switch (c) {
    case '~':
      return "$\\sim$";
    case '^':
      return "$\\hat{}$";
    case '\\':
      return "\\textbackslash ";
    case '&':
      return "\\&";
    case '%':
      return "\\%";
    case '$':
      return "\\$";
    case '#':
      return "\\#";
    case '_':
      return "\\_";
    case '{':
      return "\\{";
    case '}':
      return "\\}";
  }

  return "";
}

std::string EmitTColorBoxHeader(std::string_view color,
                                std::string_view title = "",
                                std::string_view font_color = "white") {
  if (title.empty()) {
    return StrCat("\n\\begin{tcolorbox}[colback=", color, "!5!", font_color,
                  ",colframe=", color,
                  "!75!black,left=3pt,right=3pt,enlarge top by=2mm]\n");
  } else {
    return StrCat(
        "\n\\begin{tcolorbox}[colback=", color, "!5!", font_color,
        ",colframe=", color, "!75!black,title=", title,
        ",left=3pt,right=3pt,fonttitle=\\sffamily,enlarge top by=2mm]\n");
  }
}

}  // namespace

void LatexGenerator::HandleParseTreeNode(const ParseTreeNode& node) {
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
    case ParseTreeNode::STRIKE_THROUGH:
      HandleStrikeThrough(CastNodeTypes<ParseTreeStrikeThroughNode>(node));
      break;
    case ParseTreeNode::LINK:
      HandleLink(CastNodeTypes<ParseTreeLinkNode>(node));
      break;
    case ParseTreeNode::IMAGE: {
      if (context_->GetGeneratorOptions().no_latex_image) {
        break;
      }
      HandleImage(CastNodeTypes<ParseTreeImageNode>(node));
      break;
    }
    case ParseTreeNode::TABLE:
      HandleTable(CastNodeTypes<ParseTreeTableNode>(node));
      break;
    case ParseTreeNode::VERBATIM:
      HandleVerbatim(CastNodeTypes<ParseTreeVerbatimNode>(node));
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
    case ParseTreeNode::ESCAPE:
      HandleEscape(CastNodeTypes<ParseTreeEscapeNode>(node));
      break;
    case ParseTreeNode::COMMAND:
      HandleCommand(CastNodeTypes<ParseTreeCommandNode>(node));
      break;
    case ParseTreeNode::MATH_NEWLINE:
      HandleNewlineMath(CastNodeTypes<ParseTreeMathNode>(node));
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

void LatexGenerator::EmitChar(int index) {
  if (should_escape_latex_) {
    std::string_view escaped = EscapeLatexChar(md_[index]);
    if (!escaped.empty()) {
      GetCurrentTarget()->append(escaped);
    } else
      GetCurrentTarget()->push_back(md_[index]);
  } else {
    GetCurrentTarget()->push_back(md_[index]);
  }
}

void LatexGenerator::EmitChar(int from, int to) {
  for (int i = from; i < to; i++) {
    EmitChar(i);
  }
}

void LatexGenerator::HandleParagraph(const ParseTreeParagraphNode& node) {
  if (node.Start() >= node.End()) {
    return;
  }

  GetCurrentTarget()->append("\n");
  GenerateWithDefaultAction(node, [this](int index) { EmitChar(index); });
  GetCurrentTarget()->append("\n");
}

void LatexGenerator::HandleText(const ParseTreeTextNode& node) {
  GenerateWithDefaultAction(node, [this](int index) { EmitChar(index); });
}

void LatexGenerator::HandleBold(const ParseTreeBoldNode& node) {
  GetCurrentTarget()->append("\\textbf{");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 2,
      node.End() - 2);

  GetCurrentTarget()->append("}");
}

void LatexGenerator::HandleItalic(const ParseTreeItalicNode& node) {
  GetCurrentTarget()->append("\\emph{");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 1,
      node.End() - 1);

  GetCurrentTarget()->append("}");
}

void LatexGenerator::HandleStrikeThrough(
    const ParseTreeStrikeThroughNode& node) {
  GetCurrentTarget()->append("\\sout{");

  GenerateWithDefaultActionSpan(
      node, [this](int index) { EmitChar(index); }, node.Start() + 2,
      node.End() - 2);

  GetCurrentTarget()->append("}");
}

void LatexGenerator::HandleLink(const ParseTreeLinkNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "Number of children is not two");

  links_.push_back(LatexLinkBuilder());

  targets_.push_back(&links_.back().link_desc);
  HandleParseTreeNode(*node.GetChildren()[0]);
  targets_.pop_back();

  targets_.push_back(&links_.back().link_url);
  HandleParseTreeNode(*node.GetChildren()[1]);
  targets_.pop_back();

  const LatexLinkBuilder& link = links_.back();
  GetCurrentTarget()->append(
      fmt::format("\\href{{{}}}{{{}}}", link.link_url, link.link_desc));
  links_.pop_back();
}

void LatexGenerator::HandleImage(const ParseTreeImageNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "Number of children is not two");

  const ParseTreeNode* desc_node = node.GetChildren()[0].get();
  MD2_ASSERT(desc_node->GetNodeType() == ParseTreeNode::NODE, "");

  const ParseTreeNode* desc = desc_node->GetChildren()[0].get();
  MD2_ASSERT(desc->GetNodeType() == ParseTreeNode::TEXT, "");

  images_.push_back(LatexImageBuilder());

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

  images_.back().url = GetStringInNode(node.GetChildren()[1].get(), 1, 1);

  const LatexImageBuilder& image = images_.back();
  if (image.caption.empty()) {
    GetCurrentTarget()->append(
        StrCat("\n\\begin{figure}[H]\n\\centering\n\\includegraphics[max width="
               "0.7\\linewidth]{",
               context_->FindImageForLatex(image.url), "}\n\\end{figure}\n"));
  } else {
    GetCurrentTarget()->append(
        StrCat("\n\\begin{figure}[H]\n\\centering\n\\includegraphics[max width="
               "0.7\\linewidth]{",
               context_->FindImageForLatex(image.url), "}\n\\caption*{",
               image.caption, "}\n\\end{figure}\n"));
  }

  images_.pop_back();
}

void LatexGenerator::HandleTable(const ParseTreeTableNode& node) {
  const size_t col_size = node.GetColSize();
  if (col_size == 0) {
    return;
  }

  GetCurrentTarget()->append("\n\\begin{tabularx}{\\textwidth}");
  GetCurrentTarget()->append("{|");
  for (size_t i = 0; i < col_size; i++) {
    GetCurrentTarget()->append("X|");
  }
  GetCurrentTarget()->append("}\n\\hline\n");

  // The first row is always the header.
  for (size_t i = 0; i < col_size; i++) {
    HandleParseTreeNode(*node.GetChildren()[i]);
    if (i != col_size - 1) {
      GetCurrentTarget()->append(" & ");
    }
  }
  GetCurrentTarget()->append(" \\\\ \\hline\n ");

  // The second row of the table always indicates the alignment of the cells.
  // TODO Set the alignmnent of the columns.

  for (size_t i = 2 * col_size; i < node.GetChildren().size(); i++) {
    HandleParseTreeNode(*node.GetChildren()[i]);
    if (i % col_size != col_size - 1) {
      GetCurrentTarget()->append(" & ");
    } else {
      GetCurrentTarget()->append(" \\\\ \\hline\n ");
    }
  }

  GetCurrentTarget()->append("\\end{tabularx}");
}

void LatexGenerator::HandleVerbatim(const ParseTreeVerbatimNode& node) {
  if (node.GetChildren().empty()) {
    GetCurrentTarget()->append("\\texttt{");
    EmitChar(node.Start() + 1, node.End() - 1);
    GetCurrentTarget()->append("}");

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
        StrCat("\\begin{minted}{cpp}\n", formatted_cpp, "\n\\end{minted}\n"));
  } else if (name == "py") {
    GetCurrentTarget()->append(StrCat("\\begin{minted}{python}\n",
                                      GetStringInNode(content_node.get()),
                                      "\n\\end{minted}\n"));
  } else if (name == "asm") {
    GetCurrentTarget()->append(StrCat("\\begin{minted}{nasm}\n",
                                      GetStringInNode(content_node.get()),
                                      "\n\\end{minted}\n"));
  } else if (name == "cpp-formatted") {
    GetCurrentTarget()->append(StrCat("\\begin{minted}{cpp}\n",
                                      GetStringInNode(content_node.get()),
                                      "\n\\end{minted}\n"));
  } else if (name == "compiler-warning") {
    GetCurrentTarget()->append(
        "\n\\begin{mdcompilerwarning}\n\\begin{Verbatim}[breaklines=true]\n");
    DisableLatexEscape();
    EmitChar(content_node->Start(), content_node->End());
    RestoreLatexEscape();
    GetCurrentTarget()->append("\n\\end{Verbatim}\n\\end{mdcompilerwarning}\n");
  } else if (name == "embed") {
    // Ignore.
    return;
  } else if (name == "exec") {
    GetCurrentTarget()->append(
        "\\begin{mdprogout}\n\\begin{Verbatim}[breaklines=true]\n");
    DisableLatexEscape();
    EmitChar(content_node->Start(), content_node->End());
    RestoreLatexEscape();
    GetCurrentTarget()->append("\n\\end{Verbatim}\n\\end{mdprogout}\n");
  } else if (name == "info") {
    GetCurrentTarget()->append(EmitTColorBoxHeader("green"));
    EmitChar(content_node->Start(), content_node->End());
    GetCurrentTarget()->append("\n\\end{tcolorbox}\n");
  } else if (name == "info-verb") {
    DisableLatexEscape();
    GetCurrentTarget()->append(
        "\\begin{infoverb}\n\\begin{Verbatim}[breaklines=true]\n");
    EmitChar(content_node->Start(), content_node->End());
    EmitChar(content_node->Start(), content_node->End());
    RestoreLatexEscape();
    GetCurrentTarget()->append("\n\\end{Verbatim}\n\\end{infoverb}\n");
  } else if (name == "info-term") {
    GetCurrentTarget()->append("\\begin{minted}{bash}\n");
    DisableLatexEscape();
    EmitChar(content_node->Start(), content_node->End());
    RestoreLatexEscape();
    GetCurrentTarget()->append("\n\\end{minted}");
  }
}

void LatexGenerator::HandleList(const ParseTreeListNode& node) {
  if (IsInBoxEnvironment("conditions")) {
    GetCurrentTarget()->append("\n\\begin{enumerate}[가.]");
  } else if (IsInBoxEnvironment("examples")) {
    GetCurrentTarget()->append("\n\\begin{enumerate}[ㄱ.]");
  } else if (IsInBoxEnvironment("candidates")) {
    GetCurrentTarget()->append("\n\\begin{tasks}(5)");
  } else if (node.IsOrdered()) {
    GetCurrentTarget()->append("\n\\begin{enumerate}");
  } else {
    GetCurrentTarget()->append("\n\\begin{itemize}");
  }

  for (const auto& child : node.GetChildren()) {
    HandleParseTreeNode(*child);
  }

  if (IsInBoxEnvironment("conditions")) {
    GetCurrentTarget()->append("\n\\end{enumerate}");
  } else if (IsInBoxEnvironment("examples")) {
    GetCurrentTarget()->append("\n\\end{enumerate}");
  } else if (IsInBoxEnvironment("candidates")) {
    GetCurrentTarget()->append("\n\\end{tasks}");
  } else if (node.IsOrdered()) {
    GetCurrentTarget()->append("\n\\end{enumerate}");
  } else {
    GetCurrentTarget()->append("\n\\end{itemize}");
  }
}

void LatexGenerator::HandleListItem(const ParseTreeListItemNode& node) {
  if (IsInBoxEnvironment("candidates")) {
    GetCurrentTarget()->append("\n\\task");
  } else {
    GetCurrentTarget()->append("\n\\item ");
  }

  for (const auto& child : node.GetChildren()) {
    HandleParseTreeNode(*child);
  }
}

void LatexGenerator::HandleHeader(const ParseTreeHeaderNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "");

  std::string_view header_symbol = GetStringInNode(node.GetChildren()[0].get());

  if (std::all_of(header_symbol.begin(), header_symbol.end(),
                  [](const char c) { return c == '#'; })) {
    if (header_symbol.size() == 3) {
      GetCurrentTarget()->append("\n\\subsection*{");
    } else if (header_symbol.size() == 4) {
      GetCurrentTarget()->append("\n\\subsubsection*{");
    }

    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("}\n");
    return;
  }
}

void LatexGenerator::HandleEscape(const ParseTreeEscapeNode& node) {
  EmitChar(node.Start() + 1);
}

void LatexGenerator::HandleCommand(const ParseTreeCommandNode& node) {
  std::string_view command = node.GetCommandName();
  if (command == "sidenote") {
    GetCurrentTarget()->append("\\footnote{");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("}");
  } else if (command == "sc") {
    GetCurrentTarget()->append("\\textsc{");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("}");
  } else if (command == "serif") {
    GetCurrentTarget()->append("\\emph{");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("}");
  } else if (command == "htmlonly") {
    // IGNORE
    return;
  } else if (command == "escape") {
    HandleParseTreeNode(*node.GetChildren()[0]);
  } else if (command == "footnote") {
    GetCurrentTarget()->append("\\footnote{");
    HandleParseTreeNode(*node.GetChildren()[0]);
    GetCurrentTarget()->append("}");
  } else if (command == "tooltip") {
    // IGNORE
    return;
  } else if (command == "newline") {
    GetCurrentTarget()->append("\\newline");
  } else if (command == "ref") {
    std::string_view ref_name = GetStringInNode(node.GetChildren()[0].get());
    GetCurrentTarget()->append(
        GetReferenceNodeGeneratedOutput(std::string(ref_name)));
  }
}

void LatexGenerator::HandleNewlineMath(const ParseTreeMathNode& node) {
  // Newline Math is already enclosed by \[ and \].
  DisableLatexEscape();
  EmitChar(node.Start(), node.End());
  RestoreLatexEscape();
}

void LatexGenerator::HandleMath(const ParseTreeMathNode& node) {
  DisableLatexEscape();
  EmitChar(node.Start() + 1, node.End() - 1);
  RestoreLatexEscape();
}

void LatexGenerator::HandleBox(const ParseTreeBoxNode& node) {
  MD2_ASSERT(node.GetChildren().size() == 2, "");
  const auto& box_name_node = node.GetChildren()[0];

  std::string_view box_name = md_.substr(
      box_name_node->Start(), box_name_node->End() - box_name_node->Start());

  BoxInserter box_inserter(&current_box_, box_name);

  if (box_name == "info-text") {
    GetCurrentTarget()->append(EmitTColorBoxHeader("green"));
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("\n\\end{tcolorbox}\n");
  } else if (box_name == "warning") {
    GetCurrentTarget()->append(EmitTColorBoxHeader("red"));
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("\n\\end{tcolorbox}\n");
  } else if (box_name == "lec-warning") {
    GetCurrentTarget()->append(EmitTColorBoxHeader("red", "주의 사항"));
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("\n\\end{tcolorbox}\n");
  } else if (box_name == "lec-info") {
    GetCurrentTarget()->append(EmitTColorBoxHeader("green", "참고 사항"));
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("\n\\end{tcolorbox}\n");
  } else if (box_name == "lec-summary") {
    GetCurrentTarget()->append(EmitTColorBoxHeader("blue", "뭘 배웠지?"));
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("\n\\end{tcolorbox}\n");
  } else if (box_name == "html-only") {
    // Ignore
    return;
  } else if (box_name == "latex-only") {
    HandleParseTreeNode(*node.GetChildren()[1]);
  } else if (box_name == "sidenote") {
    GetCurrentTarget()->append("\\footnote{");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("}");
  } else if (box_name == "conditions") {
    GetCurrentTarget()->append("\\begin{RectBox}[$<$ 조건 $>$ ]\n");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("\n\\end{RectBox}");
  } else if (box_name == "examples") {
    GetCurrentTarget()->append("\\begin{RectBox}[$<$ 보기 $>$ ]\n");
    HandleParseTreeNode(*node.GetChildren()[1]);
    GetCurrentTarget()->append("\n\\end{RectBox}");
  } else if (box_name == "candidates") {
    HandleParseTreeNode(*node.GetChildren()[1]);
  } else if (box_name == "note") {
    // TODO Implement the note in latex.
    return;
  } else if (box_name.substr(0, 4) == "ref-") {
    return;
  }
}

void LatexGenerator::HandleQuote(const ParseTreeQuoteNode& node) {
  GetCurrentTarget()->append("\n\\begin{displayquote}\n");
  GenerateWithDefaultAction(node, [this](int index) {
    /* Do nothing */
    (void)(index);
    (void)(this);
  });

  GetCurrentTarget()->append("\n\\end{displayquote}\n");
}

void LatexGenerator::DisableLatexEscape() {
  escape_latex.push_back(should_escape_latex_);
  should_escape_latex_ = false;
}

void LatexGenerator::RestoreLatexEscape() {
  should_escape_latex_ = escape_latex.back();
  escape_latex.pop_back();
}

bool LatexGenerator::IsInBoxEnvironment(std::string_view box_name) const {
  for (const auto& box : current_box_) {
    if (box == box_name) {
      return true;
    }
  }

  return false;
}

}  // namespace md2

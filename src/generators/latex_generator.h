#ifndef GENERATORS_LATEX_GENERATOR_H
#define GENERATORS_LATEX_GENERATOR_H

#include <iostream>
#include <vector>

#include "../parse_tree_nodes/box.h"
#include "../parse_tree_nodes/command.h"
#include "../parse_tree_nodes/escape.h"
#include "../parse_tree_nodes/header.h"
#include "../parse_tree_nodes/image.h"
#include "../parse_tree_nodes/link.h"
#include "../parse_tree_nodes/list.h"
#include "../parse_tree_nodes/paragraph.h"
#include "../parse_tree_nodes/quote.h"
#include "../parse_tree_nodes/table.h"
#include "../parse_tree_nodes/text_decoration.h"
#include "../parse_tree_nodes/verbatim.h"
#include "../string_util.h"
#include "generator.h"

namespace md2 {

struct LatexLinkBuilder {
  std::string link_desc;
  std::string link_url;
};

struct LatexImageBuilder {
  std::string alt;
  std::string caption;
  std::string size;
  std::string url;
};

class LatexGenerator : public Generator {
 public:
  LatexGenerator(std::string_view filename, std::string_view content,
                 GeneratorContext& context, const ParseTree& parse_tree)
      : Generator(filename, content, context, parse_tree) {}

 private:
  void EmitChar(int index);

  // [from, to)
  void EmitChar(int from, int to);
  void HandleParseTreeNode(const ParseTreeNode& node) override;

  void HandleParagraph(const ParseTreeParagraphNode& node);
  void HandleText(const ParseTreeTextNode& node);
  void HandleBold(const ParseTreeBoldNode& node);
  void HandleItalic(const ParseTreeItalicNode& node);
  void HandleStrikeThrough(const ParseTreeStrikeThroughNode& node);
  void HandleLink(const ParseTreeLinkNode& node);
  void HandleImage(const ParseTreeImageNode& node);
  void HandleTable(const ParseTreeTableNode& node);
  void HandleList(const ParseTreeListNode& node);
  void HandleListItem(const ParseTreeListItemNode& node);
  void HandleHeader(const ParseTreeHeaderNode& node);
  void HandleVerbatim(const ParseTreeVerbatimNode& node);
  void HandleEscape(const ParseTreeEscapeNode& node);
  void HandleCommand(const ParseTreeCommandNode& node);
  void HandleMath(const ParseTreeMathNode& node);
  void HandleBox(const ParseTreeBoxNode& node);
  void HandleQuote(const ParseTreeQuoteNode& node);

  void DisableLatexEscape();
  void RestoreLatexEscape();

 private:
  bool should_escape_latex_ = true;

  std::vector<LatexLinkBuilder> links_;
  std::vector<LatexImageBuilder> images_;

  std::vector<bool> escape_latex;
};

}  // namespace md2

#endif

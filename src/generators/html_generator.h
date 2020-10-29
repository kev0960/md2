#ifndef GENERATORS_HTML_GENERATOR_H
#define GENERATORS_HTML_GENERATOR_H

#include <iostream>
#include <vector>

#include "../parse_tree_nodes/command.h"
#include "../parse_tree_nodes/escape.h"
#include "../parse_tree_nodes/header.h"
#include "../parse_tree_nodes/image.h"
#include "../parse_tree_nodes/link.h"
#include "../parse_tree_nodes/list.h"
#include "../parse_tree_nodes/paragraph.h"
#include "../parse_tree_nodes/table.h"
#include "../parse_tree_nodes/text_decoration.h"
#include "../parse_tree_nodes/verbatim.h"
#include "../string_util.h"
#include "generator.h"

namespace md2 {

struct HTMLLinkBuilder {
  std::string link_desc;
  std::string link_url;
};

struct HTMLImageBuilder {
  std::string alt;
  std::string caption;
  std::string size;
  std::string url;
};

class HTMLGenerator : public Generator {
 public:
  HTMLGenerator(std::string_view content) : Generator(content) {}

 private:
  void HandleParseTreeNode(const ParseTreeNode& node) override;

  void HandleParagraph(const ParseTreeParagraphNode& node);
  void HandleText(const ParseTreeTextNode& node);
  void HandleBold(const ParseTreeBoldNode& node);
  void HandleItalic(const ParseTreeItalicNode& node);
  void HandleLink(const ParseTreeLinkNode& node);
  void HandleImage(const ParseTreeImageNode& node);
  void HandleTable(const ParseTreeTableNode& node);
  void HandleList(const ParseTreeListNode& node);
  void HandleListItem(const ParseTreeListItemNode& node);
  void HandleHeader(const ParseTreeHeaderNode& node);
  void HandleVerbatim(const ParseTreeVerbatimNode& node);
  void HandleEscape(const ParseTreeEscapeNode& node);
  void HandleCommand(const ParseTreeCommandNode& node);

  std::vector<HTMLLinkBuilder> links_;
  std::vector<HTMLImageBuilder> images_;
};

}  // namespace md2

#endif

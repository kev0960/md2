#ifndef GENERATORS_HWP_GENERATOR_H
#define GENERATORS_HWP_GENERATOR_H

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
#include "fmt/format.h"
#include "generator.h"
#include "hwp_state.h"

namespace md2 {

// Generator for HML (XML version of hwp) documents.
class HwpGenerator : public Generator {
 public:
  HwpGenerator(std::string_view filename, std::string_view content,
               GeneratorContext& context, const ParseTree& parse_tree)
      : Generator(filename, content, context, parse_tree) {
    hwp_state_manager_.AddDefaultMappings();
  }

  HwpStateManager& GetHwpStateManager() { return hwp_state_manager_; }

 private:
  void EmitChar(int index);

  // [from, to).
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
  void HandleNewlineMath(const ParseTreeNewlineMathNode& node);
  void HandleBox(const ParseTreeBoxNode& node);
  void HandleQuote(const ParseTreeQuoteNode& node);

  HwpStateManager hwp_state_manager_;

  int paragraph_nest_count_ = 0;

  int inst_id_ = 1;
  int z_order_ = 1;
  int bin_item_ = 1;

  class ParagraphWrapper {
   public:
    ParagraphWrapper(HwpGenerator* gen, bool wrap_text = false)
        : gen_(gen), wrap_text_(wrap_text) {
      if (gen_->paragraph_nest_count_ == 0) {
        auto [shape, style] = gen_->hwp_state_manager_.GetParaShape(
            HwpStateManager::PARA_REGULAR);

        if (wrap_text_) {
          gen_->GetCurrentTarget()->append(fmt::format(
              R"(<P ParaShape="{}" Style="{}"><TEXT CharShape="{}">)", shape,
              style,
              gen_->hwp_state_manager_.GetCharShape(
                  HwpStateManager::CHAR_REGULAR)));
        } else {
          gen_->GetCurrentTarget()->append(
              fmt::format(R"(<P ParaShape="{}" Style="{}">)", shape, style));
        }
        paragraph_added_ = true;
      }
    }

    ~ParagraphWrapper() {
      if (paragraph_added_) {
        if (wrap_text_) {
          gen_->GetCurrentTarget()->append("</TEXT></P>");
        } else {
          gen_->GetCurrentTarget()->append("</P>");
        }
      }
    }

   private:
    HwpGenerator* gen_;
    bool paragraph_added_ = false;
    bool wrap_text_ = false;
  };
};

}  // namespace md2

#endif

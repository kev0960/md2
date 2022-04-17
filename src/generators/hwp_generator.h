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
  struct HwpStatus {
    int inst_id = 1;
    int z_order = 1;
    int bin_item = 1;
  };

  HwpGenerator(std::string_view filename, std::string_view content,
               GeneratorContext& context, const ParseTree& parse_tree,
               const HwpStatus& initial_hwp_status)
      : Generator(filename, content, context, parse_tree),
        hwp_status_(initial_hwp_status) {
    hwp_state_manager_.AddDefaultMappings();
  }

  HwpStateManager& GetHwpStateManager() { return hwp_state_manager_; }
  const HwpStatus& GetHwpStatus() const { return hwp_status_; }

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
  HwpStatus hwp_status_;

  int paragraph_nest_count_ = 0;
  int total_paragraph_count_ = 0;

  enum HwpXmlTag {
    P,
    TEXT,
    CHAR,
    TABLE,
    CELL,
    PARALIST,
    ROW,
  };
  std::vector<HwpXmlTag> xml_tree_;

  bool NestedInP() const {
    for (auto itr = xml_tree_.rbegin(); itr != xml_tree_.rend(); itr++) {
      if (*itr == HwpXmlTag::P) {
        return true;
      } else if (*itr == HwpXmlTag::PARALIST) {
        // Child of PARALIST is P.
        return false;
      }
    }

    return false;
  }

  bool NestedInText() const {
    for (auto itr = xml_tree_.rbegin(); itr != xml_tree_.rend(); itr++) {
      if (*itr == HwpXmlTag::TEXT) {
        return true;
      } else if (*itr == HwpXmlTag::P) {
        // Child of PARALIST is P.
        return false;
      }
    }

    return false;
  }

  class TextWrapper {
   public:
    TextWrapper(HwpGenerator* gen, int char_shape) : gen_(gen) {
      if (!gen_->NestedInText()) {
        gen_->xml_tree_.push_back(HwpXmlTag::TEXT);

        gen_->GetCurrentTarget()->append(
            fmt::format(R"(<TEXT CharShape="{}">)", char_shape));
        text_added_ = true;
      }
    }

    ~TextWrapper() {
      if (text_added_) {
        gen_->xml_tree_.pop_back();
        gen_->GetCurrentTarget()->append("</TEXT>");
      }
    }

   private:
    HwpGenerator* gen_;
    bool text_added_ = false;
  };

  class ParagraphWrapper {
   public:
    ParagraphWrapper(HwpGenerator* gen, bool skip = false) : gen_(gen) {
      if (!skip && !gen_->NestedInP()) {
        gen_->xml_tree_.push_back(HwpXmlTag::P);

        auto [shape, style] = gen_->hwp_state_manager_.GetParaShape(
            HwpStateManager::PARA_REGULAR);
        gen_->GetCurrentTarget()->append(
            fmt::format(R"(<P ParaShape="{}" Style="{}">)", shape, style));

        paragraph_added_ = true;
      }
    }

    ~ParagraphWrapper() {
      if (paragraph_added_) {
        gen_->xml_tree_.pop_back();
        gen_->GetCurrentTarget()->append("</P>");
      }
    }

   private:
    HwpGenerator* gen_;
    bool paragraph_added_ = false;
  };
};

}  // namespace md2

#endif

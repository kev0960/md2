#ifndef GENERATORS_OBJDUMP_HIGHLIGHTER_H
#define GENERATORS_OBJDUMP_HIGHLIGHTER_H

#include "generators/generator_context.h"
#include "syntax_highlighter.h"

namespace md2 {

class ObjdumpHighlighter : public SyntaxHighlighter {
 public:
  ObjdumpHighlighter(const GeneratorContext& context, std::string_view code,
                     std::string_view language)
      : SyntaxHighlighter(code, language), context_(context) {
    class_to_style_map_.insert({"in", {{"color", "#6dbeff"}}});
    class_to_style_map_.insert({"rg", {{"color", "#78dce8"}}});
    class_to_style_map_.insert({"u", {{"color", "#b26a2f"}}});
    class_to_style_map_.insert({"lb", {{"color", "#f19798"}}});
    class_to_style_map_.insert({"dr", {{"color", "#ff3133"}}});
  }

  bool ParseCode() override;
  void HandleFunctionHeader(size_t start, size_t end);
  size_t HandleOffset(size_t start, size_t end);

  std::string GetReferenceOf(SyntaxTokenType type,
                             std::string_view snippet) const override;

 private:
  void AppendTokenVec(const std::vector<SyntaxToken>& tokens, size_t offset);

  void AppendCurrentToken(SyntaxTokenType current_token, size_t token_start,
                          size_t token_end);

  // Find the link to the assembly instruction. This will normalize the assembly
  // instruction.
  std::string_view FindAssemblyReference(std::string_view inst) const;

  const GeneratorContext& context_;
};

}  // namespace md2

#endif

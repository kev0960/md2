#include "text_decoration.h"

#include <cassert>

namespace md2 {

void ParseTreeBoldNode::Generate(Generator* generator) const {
  generator->EmitBoldStart();

  // Print any character that is not part of the child node.
  // Note that we should not print the prefix and suffix **.
  GenerateWithDefaultActionSpan(
      generator, [](Generator* g, int index) { g->Emit(index); }, start_ + 2,
      end_ - 2);

  generator->EmitBoldEnd();
}

void ParseTreeItalicNode::Generate(Generator* generator) const {
  generator->EmitItalicStart();

  // Print any character that is not part of the child node.
  GenerateWithDefaultActionSpan(
      generator, [](Generator* g, int index) { g->Emit(index); }, start_ + 1,
      end_ - 1);
  generator->EmitItalicEnd();
}

}  // namespace md2

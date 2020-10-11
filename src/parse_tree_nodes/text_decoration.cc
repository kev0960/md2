#include "text_decoration.h"

#include <cassert>

namespace md2 {

void ParseTreeBoldNode::Generate(Generator* generator) const {
  generator->EmitBoldStart();

  // Print any character that is not part of the child node.
  GenerateWithDefaultAction(generator,
                            [](Generator* g, int index) { g->Emit(index); });
  generator->EmitBoldEnd();
}

void ParseTreeItalicNode::Generate(Generator* generator) const {
  generator->EmitItalicStart();

  // Print any character that is not part of the child node.
  GenerateWithDefaultAction(generator,
                            [](Generator* g, int index) { g->Emit(index); });
  generator->EmitItalicEnd();
}

}  // namespace md2

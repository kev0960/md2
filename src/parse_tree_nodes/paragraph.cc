#include "paragraph.h"

namespace md2 {

void ParseTreeParagraphNode::Generate(Generator* generator) const {
  generator->EmitPStart();

  // Print any character that is not part of the child node.
  GenerateWithDefaultAction(generator,
                            [](Generator* g, int index) { g->Emit(index); });
  generator->EmitPEnd();
}

}  // namespace md2

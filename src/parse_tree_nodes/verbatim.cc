#include "verbatim.h"

namespace md2 {

// If the verbatim node does not have any child node, then emit the current text
// as verbatim. If it does have children, then the first node tells the type of
// code it is showing and the second node contains the actual code.
void ParseTreeVerbatimNode::Generate(Generator* generator) const {
  if (children_.empty()) {
    // Then this is the inline.
    generator->EmitInlineVerbatimStart();
    generator->Emit(start_, end_);
    generator->EmitInlineVerbatimEnd();
  }
}

}  // namespace md2

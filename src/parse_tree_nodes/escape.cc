#include "escape.h"

#include <cassert>

namespace md2 {

void ParseTreeEscapeNode::Generate(Generator* generator) const {
  // Emit the escaped character.
  generator->Emit(start_ + 1);
}

}  // namespace md2

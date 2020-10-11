#include "link.h"

namespace md2 {

void ParseTreeLinkNode::Generate(Generator* generator) const {
  generator->EmitLink();
}

}  // namespace md2

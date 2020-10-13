#include "link.h"

#include <cassert>

namespace md2 {

void ParseTreeLinkNode::Generate(Generator* generator) const {
  assert(("Number of children is not two", children_.size() == 2));

  generator->StartLink();

  generator->EmitLinkUrlStart();
  children_[0]->Generate(generator);
  generator->EmitLinkUrlEnd();

  generator->EmitLinkDescStart();
  children_[1]->Generate(generator);
  generator->EmitLinkDescEnd();

  generator->EndLink();
}

}  // namespace md2

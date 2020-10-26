#include "link.h"

#include <cassert>

namespace md2 {

void ParseTreeLinkNode::Generate(Generator* generator) const {
  assert(("Number of children is not two", children_.size() == 2));

  generator->StartLink();

  generator->EmitLinkDescStart();
  children_[0]->Generate(generator);
  generator->EmitLinkDescEnd();

  generator->EmitLinkUrlStart();
  children_[1]->Generate(generator);
  generator->EmitLinkUrlEnd();

  generator->EndLink();
}

}  // namespace md2

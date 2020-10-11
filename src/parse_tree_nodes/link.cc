#include "link.h"

namespace md2 {

void ParseTreeLinkNode::Generate(Generator* generator) const {
  generator->StartLink();

  generator->EmitLinkUrlStart();
  link_->Generate(generator);
  generator->EmitLinkUrlEnd();

  generator->EmitLinkDescStart();
  link_desc_->Generate(generator);
  generator->EmitLinkDescEnd();

  generator->EndLink();
}

}  // namespace md2

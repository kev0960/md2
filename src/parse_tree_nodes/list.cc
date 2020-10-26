#include "list.h"

namespace md2 {

void ParseTreeListNode::Generate(Generator* generator) const {
  if (is_ordered_) {
    generator->EmitOrderedListStart();
  } else {
    generator->EmitListStart();
  }

  for (auto& child : children_) {
    child->Generate(generator);
  }

  if (is_ordered_) {
    generator->EmitOrderedListEnd();
  } else {
    generator->EmitListEnd();
  }
}

void ParseTreeListItemNode::Generate(Generator* generator) const {
  if (is_ordered_) {
    generator->EmitOrderedListItemStart();
  } else {
    generator->EmitListItemStart();
  }

  for (auto& child : children_) {
    child->Generate(generator);
  }

  if (is_ordered_) {
    generator->EmitOrderedListItemEnd();
  } else {
    generator->EmitListItemEnd();
  }
}

}  // namespace md2

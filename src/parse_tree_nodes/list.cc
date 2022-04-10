#include "list.h"

namespace md2 {

void ParseTreeListNode ::SetListItemIndexes() {
  int index = 0;
  for (auto& child : GetChildren()) {
    if (child->GetNodeType() == ParseTreeNode::LIST_ITEM ||
        child->GetNodeType() == ParseTreeNode::ORDERED_LIST_ITEM) {
      ParseTreeListItemNode* list_item =
          dynamic_cast<ParseTreeListItemNode*>(child.get());
      list_item->SetListIndex(index++);
    }
  }
}

}  // namespace md2

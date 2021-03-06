#include "table.h"

namespace md2 {

void ParseTreeTableNode::SetRowSizeIfNotSpecified() {
  if (row_size_ == 0) {
    row_size_ = children_.size();
  }
}

}  // namespace md2

#include "table.h"

namespace md2 {

void ParseTreeTableNode::SetRowSizeIfNotSpecified() {
  if (col_size_ == 0) {
    col_size_ = children_.size();
  }
}

}  // namespace md2

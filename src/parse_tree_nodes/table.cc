#include "table.h"

namespace md2 {

void ParseTreeTableNode::SetRowSizeIfNotSpecified() {
  if (row_size_ == 0) {
    row_size_ = children_.size();
  }
}

void ParseTreeTableNode::Generate(Generator* generator) const {
  generator->EmitTableStart();
  generator->EmitTableHeaderStart();
  generator->EmitTableRowStart();

  // The first row is always the header.
  for (int i = 0; i < row_size_; i++) {
    generator->EmitTableHeaderCellStart();
    children_[i]->Generate(generator);
    generator->EmitTableHeaderCellEnd();
  }

  generator->EmitTableRowEnd();
  generator->EmitTableHeaderEnd();

  // The second row of the table always indicates the alignment of the cells.
  // TODO Set the alignmnent of the columns.

  generator->EmitTableBodyStart();
  for (int i = 2 * row_size_; i < children_.size(); i++) {
    if (i % row_size_ == 0) {
      generator->EmitTableRowStart();
    }

    generator->EmitTableCellStart();
    children_[i]->Generate(generator);
    generator->EmitTableCellEnd();

    if (i % row_size_ == 0) {
      generator->EmitTableRowEnd();
    }
  }

  generator->EmitTableBodyEnd();
  generator->EmitTableEnd();
}

}  // namespace md2

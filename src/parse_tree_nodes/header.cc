#include "header.h"

#include <algorithm>

namespace md2 {

void ParseTreeHeaderNode::SetHeader(std::string_view header) {
  header_ = header;

  if (std::all_of(header.begin(), header.end(),
                  [](const char c) { return c == '#'; })) {
    header_types_ = NORMAL_HEADER;
  } else if (header == "#@") {
    header_types_ = FANCY_HEADER_FOR_REF;
  } else if (header == "##@") {
    header_types_ = TEMPLATE;
  } else if (header == "###@") {
    header_types_ = LECTURE_HEADER;
  }

  header_types_ = NORMAL_HEADER;
}

}  // namespace md2


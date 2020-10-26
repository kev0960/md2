#include "parse_tree.h"

namespace md2 {

void ParseTree::Generate(Generator* generator) { root_->Generate(generator); }

}  // namespace md2

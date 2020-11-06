#ifndef GENERATORS_GENERATOR_CONTEXT_H
#define GENERATORS_GENERATOR_CONTEXT_H

#include <unordered_map>

#include "parse_tree_nodes/paragraph.h"

namespace md2 {

// Common object that is shared by generators.
class GeneratorContext {
 public:
  std::string_view GetClangFormatted(const ParseTreeTextNode* node,
                                     std::string_view md);

 private:
  std::unordered_map<const ParseTreeTextNode*, std::string>
      verbatim_to_formatted_;
};

}  // namespace md2

#endif

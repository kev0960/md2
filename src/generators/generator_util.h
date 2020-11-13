#ifndef GENERATORS_GENERATOR_UTIL_H
#define GENERATORS_GENERATOR_UTIL_H

#include <string_view>

namespace md2 {

template <typename To, typename From>
const To& CastNodeTypes(const From& node) {
  return *static_cast<const To*>(&node);
}

std::string_view StripItguruLink(std::string_view url);

}  // namespace md2

#endif

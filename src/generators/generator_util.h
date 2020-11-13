#ifndef GENERATORS_GENERATOR_UTIL_H
#define GENERATORS_GENERATOR_UTIL_H

namespace md2 {

template <typename To, typename From>
const To& CastNodeTypes(const From& node) {
  return *static_cast<const To*>(&node);
}

}

#endif

#ifndef CONTAINER_UTIL_H
#define CONTAINER_UTIL_H

namespace md2 {

template <template <typename...> class Container, typename V>
bool SetContains(const Container<V>& container, const V& value) {
  return container.find(value) != container.end();
}

}  // namespace md2

#endif

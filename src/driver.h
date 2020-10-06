#ifndef DRIVER_H
#define DRIVER_H

#include <string_view>

namespace md2 {

class Driver {
 public:
  void ParseFile(std::string_view file);
};

}  // namespace md2

#endif

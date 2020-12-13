#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>

#include "driver.h"

namespace md2 {

class ArgParse {
  public:
  static DriverOptions EmitOption(int argc, char* argv[]);
};

}  // namespace md2

#endif

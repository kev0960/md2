#ifndef MD2_ASSERT_H
#define MD2_ASSERT_H

#include <iostream>

#define MD2_ASSERT(condition, message)                                   \
  do {                                                                   \
    if (!(condition)) {                                                  \
      std::cerr << "Assertion `" #condition "` failed in " << __FILE__   \
                << " line " << __LINE__ << ": " << message << std::endl; \
      std::terminate();                                                  \
    }                                                                    \
  } while (false)

#endif

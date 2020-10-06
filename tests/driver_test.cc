#include "../src/driver.h"

#include "gtest/gtest.h"

namespace md2 {
namespace {

TEST(DriverTest, RunParseFile) {
  Driver d;
  d.ParseFile("hi");

  EXPECT_EQ(1, 1);
}

}  // namespace
}  // namespace md2

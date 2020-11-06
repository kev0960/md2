#include "driver.h"

int main() {
  md2::DriverOptions option;
  option.input_dirs.push_back("/home/jaebum/md2/md");
  //option.input_files.push_back("/home/jaebum/md2/md/inst/cpuid.md");
  option.output_dir = "./output";

  md2::Driver driver(option);
  driver.Run();
}

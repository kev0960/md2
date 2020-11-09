#include "driver.h"

int main(int argc, char* argv[]) {
  md2::DriverOptions option;
  option.input_dirs.push_back("/home/jaebum/ModooCode/md");
  option.input_dirs.push_back("/home/jaebum/ModooCode/python/md");
  // option.input_files.push_back("/home/jaebum/md2/md/dump_20.md");
  option.output_dir = "/home/jaebum/ModooCode/views/new";

  md2::Driver driver(option);
  driver.Run();
}

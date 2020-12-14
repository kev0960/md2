#include <vector>
#include <iostream>

#include "argparse.h"
#include "driver.h"

int main(int argc, char* argv[]) {
  md2::DriverOptions option = md2::ArgParse::EmitOption(argc, argv);

  std::cout << "Output :" << option.output_dir << std::endl;
  md2::Driver driver(option);
  driver.Run();
}

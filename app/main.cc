#include "driver.h"

int main(int argc, char* argv[]) {
  md2::DriverOptions option;
  option.input_dirs.push_back("/home/jaebum/ModooCode/md");
  option.input_dirs.push_back("/home/jaebum/ModooCode/python/md");
  // option.input_files.push_back("/home/jaebum/md2/md/256.md");
  option.output_dir = "/home/jaebum/ModooCode/views/new";
  option.image_path = "/home/jaebum/ModooCode/static/img";
  option.json_output_dir = "/home/jaebum/ModooCode";

  option.book_file_and_dir.push_back({"135", "/home/jaebum/book/cpp"});
  option.book_file_and_dir.push_back({"231", "/home/jaebum/book/c"});

  md2::Driver driver(option);
  driver.Run();
}

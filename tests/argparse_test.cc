#include "argparse.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "string_util.h"

namespace md2 {
namespace {

using testing::ElementsAre;
using testing::Pair;

std::vector<char*> ConstructArgvFromString(const std::vector<std::string>& s) {
  std::vector<char*> argv(s.size());
  for (int i = 0; i < s.size(); i++) {
    argv[i] = const_cast<char*>(s[i].c_str());
  }

  return argv;
}

TEST(ArgParseTest, InputDir) {
  std::string param = "./md2 -input_dirs abc";
  auto str_vec = SplitStringByCharToStringVec(param, ' ');
  auto argv = ConstructArgvFromString(str_vec);

  const DriverOptions option = ArgParse::EmitOption(argv.size(), argv.data());
  EXPECT_THAT(option.input_dirs, ElementsAre("abc"));
}

TEST(ArgParseTest, MultipleInputDir) {
  std::string param =
      R"(./md2 -input_dirs "/home/jblee/modoocode,/home/jblee/modoocode/python/md")";
  auto str_vec = SplitStringByCharToStringVec(param, ' ');
  auto argv = ConstructArgvFromString(str_vec);

  const DriverOptions option = ArgParse::EmitOption(argv.size(), argv.data());
  EXPECT_THAT(
      option.input_dirs,
      ElementsAre("/home/jblee/modoocode", "/home/jblee/modoocode/python/md"));
}

TEST(ArgParseTest, SingleOption) {
  std::string param = R"(./md2 -output_dir /home -image_path /img)";
  auto str_vec = SplitStringByCharToStringVec(param, ' ');
  auto argv = ConstructArgvFromString(str_vec);

  const DriverOptions option = ArgParse::EmitOption(argv.size(), argv.data());

  EXPECT_EQ(option.output_dir, "/home");
  EXPECT_EQ(option.image_path, "/img");
}

TEST(ArgParseTest, BooleanOption) {
  std::string param = R"(./md2 -no_html -no_latex -output_dir /home)";
  auto str_vec = SplitStringByCharToStringVec(param, ' ');
  auto argv = ConstructArgvFromString(str_vec);

  const DriverOptions option = ArgParse::EmitOption(argv.size(), argv.data());

  EXPECT_EQ(option.output_dir, "/home");
  EXPECT_EQ(option.generate_html, false);
  EXPECT_EQ(option.generate_latex, false);
}

TEST(ArgParseTest, PairOption) {
  std::string param = R"(./md2 -book_to_dir "135:/home/cpp,231:/home/c")";
  auto str_vec = SplitStringByCharToStringVec(param, ' ');
  auto argv = ConstructArgvFromString(str_vec);

  const DriverOptions option = ArgParse::EmitOption(argv.size(), argv.data());
  EXPECT_THAT(option.book_file_and_dir,
              ElementsAre(Pair("135", "/home/cpp"), Pair("231", "/home/c")));
}

}  // namespace
}  // namespace md2


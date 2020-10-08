#include "../src/parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace md2 {
namespace {

using ::testing::ElementsAre;

TEST(ParserTest, Paragraph) {
  Parser parser;

  std::string content = "a";

  ParseTree tree = parser.GenerateParseTree(content);
  auto actions = tree.FlattenTree();

  EXPECT_EQ(actions.size(), 1);
  EXPECT_THAT(actions[0], testing::ElementsAre(ParseTreeNode::EMIT_P_START,
                                               ParseTreeNode::EMIT_CHAR,
                                               ParseTreeNode::EMIT_P_END));
}

}  // namespace
}  // namespace md2

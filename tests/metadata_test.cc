#include "metadata.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace md2 {
namespace {

using ::testing::ElementsAre;

TEST(MetadataTest, SimpleMetadata) {
  std::string_view content = R"(
----------------
title : some title abc  
----------------
)";

  int end = 0;
  auto metadata_or = MetadataFactory::ParseMetadata(content, end);

  ASSERT_TRUE(metadata_or);
  EXPECT_EQ(metadata_or->GetTitle(), "some title abc");
  EXPECT_EQ(end, content.size());
}

TEST(MetadataTest, SimpleMetadata2) {
  std::string_view content = R"(
----------------
title : some title abc  
cat_title :   cat title
path :  /C++ Reference/string/string_view
publish_date :  2020-10-12
is_published : true
----------------
)";

  int end = 0;
  auto metadata_or = MetadataFactory::ParseMetadata(content, end);

  ASSERT_TRUE(metadata_or);
  EXPECT_EQ(metadata_or->GetTitle(), "some title abc");
  EXPECT_EQ(metadata_or->GetCatTitle(), "cat title");
  EXPECT_EQ(metadata_or->GetPath(), "/C++ Reference/string/string_view");
  EXPECT_EQ(metadata_or->GetPublishDate(), "2020-10-12");
  EXPECT_EQ(metadata_or->IsPublished(), true);
  EXPECT_EQ(end, content.size());
}

TEST(MetadataTest, RefNames) {
  std::string_view content = R"(
----------------
ref_name : string, std::string, std::string_view,  string_view
----------------
)";

  int end = 0;
  auto metadata_or = MetadataFactory::ParseMetadata(content, end);

  ASSERT_TRUE(metadata_or);
  EXPECT_THAT(
      metadata_or->GetRefNames(),
      ElementsAre("string", "std::string", "std::string_view", "string_view"));
  EXPECT_EQ(end, content.size());
}

}  // namespace
}  // namespace md2

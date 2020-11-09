#include "metadata.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "metadata_repo.h"

namespace md2 {
namespace {

using ::testing::ElementsAre;
using ::testing::IsNull;
using ::testing::Not;

Metadata ConstructMetadata(std::string_view content) {
  size_t end;
  auto metadata_or = MetadataFactory::ParseMetadata(content, end);
  return *metadata_or;
}

TEST(MetadataTest, SimpleMetadata) {
  std::string_view content = R"(
----------------
title : some title abc  
----------------
)";

  size_t end = 0;
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

  size_t end = 0;
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

  size_t end = 0;
  auto metadata_or = MetadataFactory::ParseMetadata(content, end);

  ASSERT_TRUE(metadata_or);
  EXPECT_THAT(
      metadata_or->GetRefNames(),
      ElementsAre("string", "std::string", "std::string_view", "string_view"));
  EXPECT_EQ(end, content.size());
}

TEST(MetadataRepoTest, ConstructRepo) {
  std::string_view content = R"(
----------------
title : a
ref_name : string, std::string, std::string_view,  string_view
path : C++
----------------
)";

  Metadata metadata1 = ConstructMetadata(content);

  std::string_view content2 = R"(
----------------
title :b 
ref_name : std::string, string_view, some_func, other_func
path : C Reference
----------------
)";

  Metadata metadata2 = ConstructMetadata(content2);

  MetadataRepo repo;
  repo.RegisterMetadata("a", metadata1);
  repo.RegisterMetadata("b", metadata2);

  const Metadata* candidate = repo.FindMetadata("std::string");
  ASSERT_THAT(candidate, Not(IsNull()));
  EXPECT_EQ(candidate->GetTitle(), "a");

  const Metadata* candidate2 = repo.FindMetadata("some_func");
  ASSERT_THAT(candidate2, Not(IsNull()));
  EXPECT_EQ(candidate2->GetTitle(), "b");

  const Metadata* candidate3 = repo.FindMetadata("not_existing");
  ASSERT_THAT(candidate3, IsNull());

  const Metadata* candidate4 = repo.FindMetadata("std::string", "C Reference");
  ASSERT_THAT(candidate4, Not(IsNull()));
  EXPECT_EQ(candidate4->GetTitle(), "b");
}

}  // namespace
}  // namespace md2

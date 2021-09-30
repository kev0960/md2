#include "../server/server.h"

#include <nlohmann/json.hpp>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace md2 {
namespace {

using json = nlohmann::json;

ConvertMarkdownToHtmlResponse Convert(const zmq::message_t& msg) {
  json js = json::parse(msg.to_string_view());

  ConvertMarkdownToHtmlResponse resp;
  resp.is_ok = js["result"].get<bool>();

  if (resp.is_ok) {
    resp.html = js["payload"].get<std::string>();
  } else {
    resp.error_msg = js["reason"].get<std::string>();
  }

  return resp;
}

TEST(ServerTest, GetResponse) {
  json request = R"({
    "request_name" : "ConvertMarkdownToHtml",
    "markdown" : "**hello**, *world!*"
  }
  )"_json;

  MarkdownServer server;
  auto response = Convert(server.HandleRequest(request));

  EXPECT_TRUE(response.is_ok);
  EXPECT_EQ(response.html,
            "<p><span class='font-weight-bold'>hello</span>, <span "
            "class='font-italic'>world!</span></p>");
}

TEST(ServerTest, FailOnInvalidRequest) {
  json request = R"({
    "request_name" : "ConvertMarkdownToHtml"
  }
  )"_json;

  MarkdownServer server;
  auto response = Convert(server.HandleRequest(request));

  EXPECT_FALSE(response.is_ok);
  EXPECT_EQ(response.error_msg, "Request does not contain 'markdown'.");
}

TEST(ServerTest, UnsupportedService) {
  json request = R"({
    "request_name" : "NotSupportedService"
  }
  )"_json;

  MarkdownServer server;
  auto response = Convert(server.HandleRequest(request));

  EXPECT_FALSE(response.is_ok);
  EXPECT_EQ(response.error_msg, "Unsupported service : NotSupportedService");
}

}  // namespace
}  // namespace md2

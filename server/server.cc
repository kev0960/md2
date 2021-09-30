#include "server.h"

#include <unordered_set>

#include "generators/html_generator.h"
#include "metadata_repo.h"
#include "server_util.h"

namespace md2 {
namespace {

static const std::unordered_set<std::string> kSupportedServices = {
    "ConvertMarkdownToHtml"};

}

zmq::message_t MarkdownServer::HandleRequest(const json& request) {
  if (!request.count("request_name")) {
    return CreateError("Request name is missing.");
  }

  std::string request_name = request["request_name"].get<std::string>();
  if (!IsSupportedServce(request_name)) {
    return CreateError("Unsupported service : " + request_name);
  }

  md2::ConvertMarkdownToHtmlResponse response = ConvertMarkdownToHtml(request);

  if (response.is_ok) {
    json rep;
    rep["result"] = true;
    rep["payload"] = response.html;

    return zmq::message_t(rep.dump());
  } else {
    return CreateError(response.error_msg);
  }
}

bool MarkdownServer::IsSupportedServce(const std::string& service_name) const {
  return kSupportedServices.find(service_name) != kSupportedServices.end();
}

ConvertMarkdownToHtmlResponse MarkdownServer::ConvertMarkdownToHtml(
    const json& request) {
  if (!request.count("markdown")) {
    ConvertMarkdownToHtmlResponse response;
    response.is_ok = false;
    response.error_msg = "Request does not contain 'markdown'.";

    return response;
  }

  const std::string md = request["markdown"].get<std::string>();
  const ParseTree tree = parser_.GenerateParseTree(md);

  HTMLGenerator generator("", md, generator_context_, tree);

  ConvertMarkdownToHtmlResponse response;
  response.html = generator.Generate();
  response.is_ok = true;

  return response;
}

}  // namespace md2

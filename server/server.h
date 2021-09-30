#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <nlohmann/json.hpp>
#include <string_view>

#include "generators/generator_context.h"
#include "parser.h"

namespace md2 {

using json = nlohmann::json;

struct ConvertMarkdownToHtmlResponse {
  std::string html;

  bool is_ok;
  std::string error_msg;
};

class MarkdownServer {
 public:
  MarkdownServer()
      : generator_context_(metadata_repo_, image_dir_path_,
                           /*use_clang_server=*/false,
                           /*clang_server_port=*/-1,
                           /*context=*/nullptr) {}

  // Handles the request.
  zmq::message_t HandleRequest(const json& request);

 private:
  bool IsSupportedServce(const std::string& service_name) const;
  ConvertMarkdownToHtmlResponse ConvertMarkdownToHtml(const json& request);

  Parser parser_;
  MetadataRepo metadata_repo_;

  const std::string image_dir_path_ = "";
  GeneratorContext generator_context_;
};

}  // namespace md2

#endif

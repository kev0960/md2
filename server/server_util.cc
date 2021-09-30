#include "server_util.h"

#include <nlohmann/json.hpp>

namespace md2 {

using json = nlohmann::json;

zmq::message_t CreateError(std::string_view error) {
  json error_response;
  error_response["result"] = false;
  error_response["reason"] = error;

  return zmq::message_t(error_response.dump());
}

}  // namespace md2

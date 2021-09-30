#ifndef SERVER_SERVER_UTIL_H
#define SERVER_SERVER_UTIL_H

#include <nlohmann/json.hpp>
#include <zmq.hpp>

namespace md2 {

using json = nlohmann::json;

zmq::message_t CreateError(std::string_view error); 

}  // namespace md2

#endif

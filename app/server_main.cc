#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <zmq.hpp>

#include "server.h"
#include "server_util.h"

using json = nlohmann::json;

int main() {
  zmq::context_t context;

  zmq::socket_t socket(context, ZMQ_REP);
  socket.bind("tcp://*:5555");

  md2::MarkdownServer server;
  while (true) {
    zmq::message_t request_message;
    auto received_size = socket.recv(request_message);

    if (!received_size.has_value() || received_size.value() == 0) {
      std::cerr << "Empty request received." << std::endl;
      socket.send(md2::CreateError("Empty request received."),
                  zmq::send_flags::none);

      continue;
    }

    try {
      json request = json::parse(request_message.to_string_view());
      socket.send(server.HandleRequest(request), zmq::send_flags::none);
    } catch (std::exception& e) {
      socket.send(md2::CreateError(e.what()), zmq::send_flags::none);
    }
  }
}


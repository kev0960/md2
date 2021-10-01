#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <zmq.hpp>
#include <fmt/format.h>

#include "argparse.h"
#include "server.h"
#include "server_util.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
  md2::DriverOptions option = md2::ArgParse::EmitOption(argc, argv);

  zmq::context_t context;

  zmq::socket_t socket(context, ZMQ_REP);

  std::string connection_str = fmt::format("tcp://*:{}", option.md2_server_port);
  std::cout << "*********************************" << std::endl;
  std::cout << "*   Md2 Server running at " << option.md2_server_port << "  *" << std::endl;
  std::cout << "*********************************" << std::endl;
  socket.bind(connection_str);

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


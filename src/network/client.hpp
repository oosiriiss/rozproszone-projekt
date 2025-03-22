#pragma once
#include "socket.hpp"

namespace network {
struct Client {

public:
  Client();
  ~Client();

  bool connect(const char *ipAddress, unsigned short port);
  std::optional<SocketError> send(const char *msg, size_t len);
  std::optional<SocketError> receive();
  std::optional<std::string> pollMessage(std::string_view separator);

  // client scoket description
  Socket m_socket;
};
}; // namespace network

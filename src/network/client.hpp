#pragma once
#include "packet.hpp"
#include "socket.hpp"

namespace network {
struct Client {

public:
  Client();
  ~Client();

  bool connect(const char *ipAddress, unsigned short port);
  std::optional<SocketError> send(network::ClientPacket);
  std::optional<network::ServerPacket> pollMessage();

private:
  std::optional<SocketError> receive();
  // client scoket description
  Socket m_socket;
};
}; // namespace network

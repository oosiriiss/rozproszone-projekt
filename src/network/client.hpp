#pragma once
#include "packet.hpp"
#include "socket.hpp"
#include <queue>

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
  std::priority_queue<
      internal::PacketWrapper<network::ServerPacket>,
      std::vector<internal::PacketWrapper<network::ServerPacket>>,
      internal::PacketCompare<network::ServerPacket>>
      m_incomingPackets;
  // client scoket description
  Socket m_socket;
};
}; // namespace network

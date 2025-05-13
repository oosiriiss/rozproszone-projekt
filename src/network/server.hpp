#pragma once
#include "packet.hpp"
#include "socket.hpp"
#include <netinet/in.h>
#include <queue>
#include <vector>

namespace network {

struct Server {

  Server();
  ~Server();

  /**
   * Initializes the server socket.
   * !! Must be called before trying to use any other function !!
   **/
  bool bind(const char *ipAddress, unsigned short port);
  bool tryAcceptClient();
  //
  std::optional<std::pair<Socket *, network::ClientPacket>> pollMessage();
  std::optional<SocketError> sendAll(network::ServerPacket packet);
  std::optional<SocketError> sendOthers(const Socket *client,
                                        network::ServerPacket packet);
  std::optional<SocketError> send(Socket *client, network::ServerPacket packet);

  // Read all pending data from sockets

  const std::vector<Socket> &getClients() { return m_clients; }
  void setOnDisconnectCallback(std::function<void(int32_t)> cb);

private:
  struct ClientMessage {
    Socket *socket;
    internal::PacketWrapper<network::ClientPacket> packet;

    struct Comparator {
      bool operator()(const ClientMessage &a, const ClientMessage &b) {
        auto x = internal::PacketCompare<network::ClientPacket>();
        return x(a.packet, b.packet);
      }
    };
  };

  std::optional<SocketError> receive();

  std::priority_queue<ClientMessage, std::vector<ClientMessage>,
                      ClientMessage::Comparator>
      m_incomingPackets;

  Socket m_socket;

  std::vector<Socket> m_clients;
  std::function<void(int32_t)> m_onDisconnect;
};
} // namespace network

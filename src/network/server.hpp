#pragma once
#include "packet.hpp"
#include "socket.hpp"
#include <netinet/in.h>
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

private:
  std::optional<SocketError> receive();

  Socket m_socket;

  std::vector<Socket> m_clients;
};
} // namespace network

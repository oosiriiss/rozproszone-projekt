#pragma once
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

  // Sets the socket into blocking mode and waits for {clients} clients to
  // connect and then adds them to internal clients list
  bool waitForClients(uint32_t clients);
  std::optional<SocketError> send(const char *msg);
  std::string receive();

  Socket m_socket;

  std::vector<Socket> m_clients;
};
} // namespace network

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../logging.hpp"
#include "network.hpp"
#include "server.hpp"

namespace network {

Server::Server() : m_socketfd(INVALID_SOCKET), m_clients({}) {}

Server::~Server() {

  if (m_socketfd != INVALID_SOCKET) {
    if (shutdown(m_socketfd, SHUT_RDWR) < 0)
      LOG_ERROR("Couldn't shutdown socket ", m_socketfd);
    else
      LOG_INFO("Socket ", m_socketfd, " successfully closed");
  }
}

bool Server::bind(const char *ipAddress, unsigned short port) {

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (!inet_pton(AF_INET, ipAddress, &addr.sin_addr)) {
    LOG_ERROR("Couldn't convert addr ", ipAddress, " and port ", port);
    return false;
  }

  // AF_INET = IPv4
  constexpr int SOCKET_DOMAIN = AF_INET;
  // SOCK_STREAM = TCP, SOCK_DGRAM = UDP
  constexpr int SOCKET_TYPE = SOCK_STREAM;
  // i.e make socket nonblocking
  constexpr int SOCKET_FLAGS = 0;

  int socketfd = socket(AF_INET, SOCK_STREAM, SOCKET_FLAGS);
  if (socketfd < 0) {
    LOG_ERROR("Couldn't create socket");
    return false;
  }

  m_socketfd = socketfd;

  // Binding the server
  if (::bind(m_socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

    LOG_ERROR("Couldn't bind server socket addr: ", ipAddress, " and port ",
              port);

    return false;
  }

  // default 4 clients
  if (listen(m_socketfd, 4) < 0) {
    LOG_ERROR("listen failed");
    return false;
  }

  return true;
}

bool Server::acceptClient() {

  struct sockaddr_in client = {0};
  socklen_t len = 0;

  const int clientSocket = accept(m_socketfd, (struct sockaddr *)&client, &len);

  if (clientSocket < 0) {
    LOG_ERROR("Couldn't accept client errno: ", errno);
    return false;
  }

  m_clients.push_back(
      ClientData{.socketfd = clientSocket, .connectionData = client});

  return true;
}

bool Server::send(const char *msg) {
  if (m_socketfd == INVALID_SOCKET) {
    LOG_ERROR("Trying to use uninitialized Client");
    return false;
  }

  for (const ClientData &client : m_clients) {
    if (::send(client.socketfd, msg, strlen(msg), 0) < 0) {
      LOG_ERROR("Couldn't send message errno: ", errno);
      return false;
    }
  }

  return true;
}

bool Server::receive(char *outbuf) {
  if (m_socketfd == INVALID_SOCKET) {
    LOG_ERROR("Trying to use uninitialized Client");
    return false;
  }

  for (const ClientData &client : m_clients) {
    if (recv(client.socketfd, outbuf, 512, 0) < 0) {
      outbuf[0] = 0;
      LOG_ERROR("Couldn't receive message errno: ", errno);
      return false;
    }
  }
  return true;
}

} // namespace network

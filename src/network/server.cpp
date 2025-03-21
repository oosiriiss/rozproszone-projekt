#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <string_view>
#include <sys/socket.h>
#include <vector>

#include "../logging.hpp"
#include "server.hpp"
#include "socket.hpp"

namespace network {

Server::Server() : m_socket(Socket::NULL_SOCKET), m_clients({}) {}

Server::~Server() { m_socket.shutdown(); }

bool Server::bind(const char *ipAddress, unsigned short port) {

  auto result = Socket::create(ipAddress, port, SocketType::TCP, 0);

  if (!result)
    return false;

  m_socket = result.value();

  // Binding the server
  if (::bind(m_socket.fd, (struct sockaddr *)&m_socket.addr, m_socket.addrlen) <
      0) {

    LOG_ERROR("Couldn't bind server socket addr: ", ipAddress, " and port ",
              port);
    return false;
  }

  // default 4 clients
  if (listen(m_socket.fd, 4) < 0) {
    LOG_ERROR("listen failed");
    return false;
  }

  // Settting the socket to nonblocking mode
  if (!m_socket.setBlocking(false))
    return false;

  return true;
}

bool Server::waitForClients(uint32_t clients) {

  m_socket.setBlocking(true);

  for (int i = 0; i < clients; ++i) {

    struct sockaddr_in client = {0};
    socklen_t len = 0;

    const int clientSocket =
        accept(m_socket.fd, (struct sockaddr *)&client, &len);

    if (clientSocket < 0) {
      LOG_ERROR("Couldn't accept client errno: ", errno);
      return false;
    }

    m_clients.push_back(
        Socket{.fd = clientSocket, .addr = client, .addrlen = len});
  }
  m_socket.setBlocking(true);
  return true;
}

std::optional<SocketError> Server::send(const char *msg) {
  int clients = m_clients.size();

  for (int i = 0; i < clients; ++i) {

    Socket &client = m_clients[i];

    auto err = client.send(msg, strlen(msg));
    if (err) {
      if (err == SocketError::Disconnected) {
        LOG_INFO("Client disconnected");
        m_clients.erase(m_clients.begin() + i);
        // retry of the same iteration
        --clients;
        --i;
      }

      return *err;
    }
    LOG_INFO("AFTR");
  }
  return std::nullopt;
}

std::optional<SocketError> Server::receive() {
  for (Socket &client : m_clients) {
    auto error = client.receive();
    if (error)
      return error;
  }
  return std::nullopt;
}

bool Server::pollMessage(std::string &msg, std::string_view separator) {

  for (auto &client : m_clients) {

    if (client.hasMessage(separator)) {
      msg = client.nextMessage(separator);
      assert(msg != "" && msg != "ERROR" && msg != "NOSIZE");

      return true;
    }
  }

  return false;
}

} // namespace network

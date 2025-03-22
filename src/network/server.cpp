#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <string_view>
#include <sys/socket.h>
#include <utility>
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

    LOG_INFO("Waiting for connect");
    const int clientSocket =
        accept(m_socket.fd, (struct sockaddr *)&client, &len);
    LOG_INFO("One connected");

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

std::optional<SocketError> Server::send(const char *msg, size_t len) {
  int clients = m_clients.size();

  for (int i = 0; i < clients; ++i) {

    Socket &client = m_clients[i];

    auto err = client.send(msg, len);
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

std::optional<std::pair<Socket *, std::string>>
Server::pollMessage(std::string_view separator) {
  for (auto &client : m_clients) {

    if (auto x = client.nextMessage(separator)) {
      return std::make_pair(&client, *x);
    }
  }

  return std::nullopt;
}

} // namespace network

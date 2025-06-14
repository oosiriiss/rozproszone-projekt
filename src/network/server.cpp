#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <queue>
#include <sys/socket.h>
#include <utility>
#include <vector>

#include "../logging.hpp"
#include "packet.hpp"
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

bool Server::tryAcceptClient() {
  auto result = m_socket.accept();

  if (result.has_value()) {
    Socket s = *result;
    m_clients.push_back(s);
    s.setBlocking(false);
    return true;
  }
  return false;
}

std::optional<SocketError> Server::sendAll(network::ServerPacket packet) {
  auto msg = encodePacket(packet);
  size_t len = msg.size();

  int clients = m_clients.size();

  for (int i = clients - 1; i >= 0; --i) {

    Socket &client = m_clients[i];

    auto err = client.send(msg.c_str(), len);

    if (err) {
      LOG_ERROR("SendAll client error", std::to_underlying(*err));
      if (err == SocketError::Disconnected) {
        LOG_INFO("Client disconnected");
        m_clients.erase(m_clients.begin() + i);
        m_onDisconnect(client.fd);
      }
      return *err;
    }
  }
  return std::nullopt;
}
std::optional<SocketError> Server::sendOthers(const Socket *client,
                                              network::ServerPacket packet) {
  auto msg = encodePacket(packet);
  size_t len = msg.size();

  int clients = m_clients.size();
  for (int i = clients - 1; i >= 0; --i) {
    Socket &current = m_clients[i];

    if (client->fd == current.fd)
      continue;

    auto err = current.send(msg.c_str(), len);
    if (err) {
      LOG_ERROR("SendOthers client error", std::to_underlying(*err));
      if (err == SocketError::Disconnected) {
        LOG_INFO("Client disconnected");
        m_clients.erase(m_clients.begin() + i);
        m_onDisconnect(current.fd);
      }
      return *err;
    }
  }
  return std::nullopt;
}

void Server::setOnDisconnectCallback(std::function<void(int32_t)> cb) {
  m_onDisconnect = cb;
}

std::optional<SocketError> Server::send(Socket *client,
                                        network::ServerPacket packet) {
  auto msg = encodePacket(packet);
  size_t len = msg.size();

  auto err = client->send(msg.c_str(), len);
  if (err) {
    if (err == SocketError::Disconnected) {
      LOG_ERROR(
          "Client is disconnected cannot send IT IS NOT REMOVED from clients");
      m_clients.resize(std::distance(
          m_clients.begin(),
          std::remove_if(m_clients.begin(), m_clients.end(),
                         [client](Socket s) { return s.fd == client->fd; })));
      m_onDisconnect(client->fd);
    }

    return *err;
  }

  return std::nullopt;
}

std::optional<SocketError> Server::receive() {
  for (Socket &client : m_clients) {
    auto e = client.receive();
    if (e)
      return e;

    auto packet = client.nextMessage<network::ClientPacket>();

    if (!packet)
      continue;

    m_incomingPackets.push(ClientMessage{.socket = &client, .packet = *packet});
  }
  return std::nullopt;
}

std::optional<std::pair<Socket *, network::ClientPacket>>
Server::pollMessage() {
  if (receive()) {
    LOG_ERROR("Receive failed");
  }

  if (m_incomingPackets.empty()) {
    return std::nullopt;
  }

  const auto packet = m_incomingPackets.top();
  m_incomingPackets.pop();

  return std::make_pair(packet.socket, packet.packet.body);
}

} // namespace network

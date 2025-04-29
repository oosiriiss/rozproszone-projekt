#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <sys/socket.h>
#include <sys/types.h>

#include "../logging.hpp"
#include "client.hpp"
#include "packet.hpp"
#include "socket.hpp"

namespace network {

Client::Client() : m_socket(Socket::NULL_SOCKET) {}

Client::~Client() {
  auto error = m_socket.shutdown();
  if (error) {
    LOG_ERROR("Couldn't shutdown socket");
  }
}

bool Client::connect(const char *ipAddress, unsigned short port) {
  auto result = Socket::create(ipAddress, port, SocketType::TCP, 0);

  if (!result)
    return false;

  Socket s = result.value();
  // Connecting to the server
  if (::connect(s.fd, (const struct sockaddr *)&s.addr, s.addrlen) < 0) {
    LOG_ERROR("Couldn't connect to ", ipAddress, " with port ", port);
    return false;
  }

  // Setting the socket to run in non-blocking mode
  m_socket = s;
  if (!m_socket.setBlocking(false))
    return false;

  return true;
}

std::optional<SocketError> Client::send(network::ClientPacket packet) {

  auto msg = encodePacket(packet);
  size_t len = msg.size();

  auto error = m_socket.send(msg.c_str(), len);
  if (error) {
    return error.value();
  }

  return std::nullopt;
}

std::optional<SocketError> Client::receive() {
  auto error = m_socket.receive();
  if (error)
    return error;

  return std::nullopt;
}

std::optional<network::ServerPacket> Client::pollMessage() {
  LOG_DEBUG("Polling client");
  if (receive()) {
    LOG_ERROR("Receive failed");
  }

  auto msg = m_socket.nextMessage();

  if (!msg.has_value())
    return std::nullopt;

  auto packet = network::decodePacket<network::ServerPacket>(*msg);

  if (!packet.has_value()) {
    LOG_ERROR("Couldn't decode server packet");
    return std::nullopt;
  }

  return packet;
}

} // namespace network

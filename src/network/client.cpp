#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>

#include "../logging.hpp"
#include "client.hpp"
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

std::optional<SocketError> Client::send(const char *msg) {
  auto error = m_socket.send(msg, strlen(msg));
  if (error) {
    return error.value();
  }

  return std::nullopt;
}

std::string Client::receive() {

  // Reading the stream
  auto error = m_socket.receive();
  if (error) {
    LOG_ERROR("Couldn't receive message", static_cast<int>(error.value()),
              " Socket ", m_socket.fd);
    return "ERRSR";
  }
  // gettin the msg from stream
  auto x = m_socket.nextMessage("SEPARATOR");
  return x;
}

} // namespace network

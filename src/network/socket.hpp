#pragma once

#include <expected>

#include <netinet/in.h>
#include <optional>
#include <string>
#include <string_view>
#include <sys/socket.h>

namespace network {

enum class SocketError {
  InvalidAddress,
  NoAccess,
  InvalidDescriptor,
  NotConnected,
  WouldBlock,
  Disconnected,
  UnknownError,
};

enum class SocketType { TCP, UDP };

struct Socket {
  int32_t fd;
  struct sockaddr_in addr;
  socklen_t addrlen;

  // Holds all data read from socket
  // To read individual messages use "nextMessage"
  std::string currentData;
  ;

  [[nodiscard]] static std::expected<Socket, SocketError>
  create(const char *addr, uint16_t port, SocketType type = SocketType::TCP,
         int socketFlags = 0) noexcept;

  std::optional<SocketError> shutdown() noexcept;
  std::optional<SocketError> send(const char *msg, uint32_t msglen);
  std::optional<SocketError> receive();

  // valid only if the socket is server
  std::expected<Socket, SocketError> accept();

  std::optional<std::string> nextMessage();

  bool setBlocking(bool shouldBlock);

  static const Socket NULL_SOCKET;
};
} // namespace network

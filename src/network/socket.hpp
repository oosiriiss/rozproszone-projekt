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

  /*
   * Creates socket using IPv4 protocol of type TCP or UDP
   * prints the error message using LOG_ERROR
   */
  [[nodiscard]] static std::expected<Socket, SocketError>
  create(const char *addr, uint16_t port, SocketType type = SocketType::TCP,
         int socketFlags = 0) noexcept;

  std::optional<SocketError> shutdown() noexcept;
  std::optional<SocketError> send(const char *msg, uint32_t msglen);
  std::optional<SocketError> receive();

  // ????? is this the right approach ????
  // User should manually use receive to read the data from the socket and then
  // use nextMessage returns all the bytes unitl separator or empty string if
  // none is available
  //
  // You can check if message is avaiable with Socket::hasMessage
  //
  // Allocates new buffer for current message <-- ??? is this good
  // Returns currnet message, and moves the buffer to the next one
  std::string nextMessage(std::string_view separator);
  // Returns true if there is full message available to read
  bool hasMessage(std::string_view separator);

  bool setBlocking(bool shouldBlock);

  static const Socket NULL_SOCKET;
};
} // namespace network

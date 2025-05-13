#pragma once

#include <expected>

#include "packet.hpp"
#include <netinet/in.h>
#include <optional>
#include <string>
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

  template <typename T>
  std::optional<internal::PacketWrapper<T>> nextMessage() {

    constexpr auto separator = network::internal::SEPARATOR;
    constexpr auto separatorSize = sizeof(network::internal::SEPARATOR);

    if (this->currentData.size() <= separatorSize)
      return std::nullopt;

    const int sepIndex = this->currentData.find(separator, 0, separatorSize);
    if (sepIndex == std::string::npos) {
      LOG_ERROR("No separator found in (", std::string(separator, 4), ")");
      return std::nullopt;
    }

    LOG_DEBUG("Sep message: ", sepIndex);

    // Copying the message
    // Starting is also the length of current message
    std::string msg = std::string(this->currentData.substr(0, sepIndex));

    // Moving the internal buffer to the next message
    const auto startOfNextMessage = sepIndex + separatorSize;
    this->currentData.erase(0, startOfNextMessage);

    auto decoded = decodePacket<T>(msg);

    if (!decoded)
      return std::nullopt;

    return *decoded;
  }

  bool setBlocking(bool shouldBlock);

  static const Socket NULL_SOCKET;
};
} // namespace network

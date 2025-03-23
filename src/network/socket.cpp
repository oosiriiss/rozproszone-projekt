#include "socket.hpp"
#include "../logging.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <string_view>
#include <sys/socket.h>

namespace network {

constexpr Socket Socket::NULL_SOCKET = {
    .fd = -1, .addr = {}, .addrlen = 0, .currentData = ""};

SocketError errnoToSocketError() {

  // For more portability
  const int WOULDBLOCK = EAGAIN | EWOULDBLOCK;

  switch (errno) {
  case EPIPE:
    return SocketError::Disconnected;
  case EACCES:
    return SocketError::NoAccess;
  case EBADF:
    return SocketError::InvalidDescriptor;
  case ENOTCONN:
    return SocketError::NotConnected;
  case WOULDBLOCK:
    return SocketError::WouldBlock;
  default:
    return SocketError::UnknownError;
  }
}

constexpr int INVALID_SOCKET_DESCRIPTOR = -1;

std::expected<Socket, SocketError> Socket::create(const char *ipAddress,
                                                  uint16_t port,
                                                  SocketType type,
                                                  int socketFlags) noexcept {
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (!inet_pton(AF_INET, ipAddress, &addr.sin_addr)) {
    LOG_ERROR("Couldn't convert addr ", ipAddress, " and port ", port);
    return std::unexpected(SocketError::InvalidAddress);
  }

  // AF_INET = IPv4
  const int32_t SOCKET_DOMAIN = AF_INET;
  // SOCK_STREAM = TCP, SOCK_DGRAM = UDP
  const int32_t SOCKET_TYPE =
      (type == SocketType::TCP) ? SOCK_STREAM : SOCK_DGRAM;
  // i.e make socket nonblocking
  const int32_t SOCKET_FLAGS = socketFlags;

  int32_t socketfd = socket(SOCKET_DOMAIN, SOCKET_TYPE, SOCKET_FLAGS);

  if (socketfd < 0) {
    if (errno == EACCES) {
      LOG_ERROR("No access to create socket");
      return std::unexpected(SocketError::NoAccess);
    } else {
      LOG_ERROR("Unknown error encountered. errno: ", errno);
      return std::unexpected(SocketError::UnknownError);
    }
  }

  return Socket{.fd = socketfd,
                .addr = addr,
                .addrlen = sizeof(addr),
                .currentData = std::string(0, 0)};
}
std::optional<SocketError> Socket::shutdown() noexcept {
  // Should this be an error or just ignored?
  if (this->fd == INVALID_SOCKET_DESCRIPTOR)
    return SocketError::InvalidDescriptor;

  if (::shutdown(this->fd, SHUT_RDWR) < 0)
    return errnoToSocketError();

  return std::nullopt;
}

std::optional<SocketError> Socket::send(const char *msg, uint32_t msglen) {

  int r = ::send(this->fd, msg, msglen, MSG_NOSIGNAL);

  // Is this necessary?
  if (r < 0) {
    LOG_ERROR("SEND ERROR");
    return errnoToSocketError();
  }
  return std::nullopt;
}

std::optional<SocketError> Socket::receive() {
  // TODO :: Make this read whole socket data
  // Is this necessary?

  if (this->fd == INVALID_SOCKET_DESCRIPTOR)
    return SocketError::InvalidDescriptor;
  std::string buf(4096, 0);

  int bytesRead = recv(this->fd, &buf[0], buf.size(), 0);

  if (bytesRead <= 0)
    return errnoToSocketError();

  this->currentData.append(buf.substr(0, bytesRead));
  return std::nullopt;
}

// TODO :: MERGe this with receive
std::optional<std::string> Socket::nextMessage(std::string_view separator) {
  if (this->currentData.size() <= separator.size())
    return std::nullopt;

  const int startIndex =
      this->currentData.find(separator.data(), 0, separator.size());
  if (startIndex == std::string::npos) {
    LOG_ERROR("NO SEPARATOR FOUND IN ", currentData.size());
    return std::nullopt;
  }

  // Copying the message
  // Starting is also the length of current message
  std::string msg = std::string(this->currentData.substr(0, startIndex));

  // Moving the internal buffer to the next message
  const auto startOfNextMessage = startIndex + separator.size();
  this->currentData.erase(0, startOfNextMessage);

  return msg;
}

bool Socket::setBlocking(bool shouldBlock) {
  // TODO :: add errors for this function
  //
  int oldf = fcntl(this->fd, F_GETFL);

  if (oldf == -1) {
    return false;
  }

  int newf = (shouldBlock) ? oldf & ~O_NONBLOCK : oldf | O_NONBLOCK;

  if (fcntl(this->fd, F_SETFL, newf) == -1)
    return false;

  return true;
}

} // namespace network

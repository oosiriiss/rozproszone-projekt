#include "socket.hpp"
#include "../logging.hpp"
#include "packet.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <optional>
#include <string>
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

SocketError printSocketError(SocketError value) {
  static const auto strings = [] {
    std::map<SocketError, std::string_view> result;
#define INSERT_ELEMENT(p) result.emplace(p, #p);
    INSERT_ELEMENT(SocketError::InvalidAddress);
    INSERT_ELEMENT(SocketError::NoAccess);
    INSERT_ELEMENT(SocketError::InvalidDescriptor);
    INSERT_ELEMENT(SocketError::NotConnected);
    INSERT_ELEMENT(SocketError::WouldBlock);
    INSERT_ELEMENT(SocketError::Disconnected);
    INSERT_ELEMENT(SocketError::UnknownError);
#undef INSERT_ELEMENT
    return result;
  }();

  std::cout << strings.at(value);
  return value;
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

  if (this->fd == INVALID_SOCKET_DESCRIPTOR) {
    LOG_ERROR("Invalid socket descriptor");
    return SocketError::InvalidDescriptor;
  }
  std::string buf(4096, 0);

  int bytesRead = 0;

  while (int bytesRead = recv(this->fd, &buf[0], buf.size(), 0)) {
    if (bytesRead <= 0) {
      SocketError e = errnoToSocketError();
      if (e == SocketError::WouldBlock)
        return std::nullopt;
      return printSocketError(e);
    }

    this->currentData.append(buf.substr(0, bytesRead));
  }

  return std::nullopt;
}

std::expected<Socket, SocketError> Socket::accept() {
  //
  // TODO :: Move this to socket class
  //
  struct sockaddr_in client = {0};
  socklen_t len = 0;

  const int clientSocket = ::accept(this->fd, (struct sockaddr *)&client, &len);

  if (clientSocket < 0) {
    SocketError e = errnoToSocketError();
    if (e != SocketError::WouldBlock)
      printSocketError(e);
    return std::unexpected(e);
  }
  // Nonblocking socket

  return Socket{.fd = clientSocket, .addr = client, .addrlen = len};
}

// TODO :: MERGe this with receive
std::optional<std::string> Socket::nextMessage() {

  constexpr auto separator = network::internal::SEPARATOR;
  constexpr auto separatorSize = sizeof(network::internal::SEPARATOR);

  if (this->currentData.size() <= separatorSize)
    return std::nullopt;

  const int startIndex = this->currentData.find(separator, 0, separatorSize);
  if (startIndex == std::string::npos) {
    LOG_ERROR("No separator found in (", std::string(separator, 4), ")");
    return std::nullopt;
  }

  // Copying the message
  // Starting is also the length of current message
  std::string msg = std::string(this->currentData.substr(0, startIndex));

  DEBUG_ONLY(printBytes(msg));

  // Moving the internal buffer to the next message
  const auto startOfNextMessage = startIndex + separatorSize;
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

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../logging.hpp"
#include "client.hpp"
#include "network.hpp"

namespace network {
Client::Client() : m_socketfd(INVALID_SOCKET) {}

Client::~Client() {
  if (m_socketfd != INVALID_SOCKET) {
    if (shutdown(m_socketfd, SHUT_RDWR) < 0)
      LOG_ERROR("Couldn't shutdown socket errno: ", errno);
    else
      LOG_INFO("Socket ", m_socketfd, " successfully closed");
  }
}

bool Client::connect(const char *ipAddress, unsigned short port) {
  struct sockaddr_in serv = {0};

  serv.sin_family = AF_INET;
  serv.sin_port = htons(port);
  if (!inet_pton(AF_INET, ipAddress, &serv.sin_addr)) {
    LOG_ERROR("Couldn't convert addr ", ipAddress, " and port ", port);
    return false;
  }

  const int sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock < 0) {
    LOG_ERROR("Couldn't create socket");
    return false;
  }

  m_socketfd = sock;

  if (::connect(m_socketfd, (const struct sockaddr *)&serv, sizeof(serv)) < 0) {
    LOG_ERROR("Couldn't connect to ", ipAddress, " with port ", port);
    return false;
  }

  return true;
}

bool Client::send(const char *msg) {
  if (m_socketfd == INVALID_SOCKET) {
    LOG_ERROR("Trying to use uninitialized Client");
    return false;
  }

  if (::send(m_socketfd, msg, strlen(msg), 0) < 0) {
    LOG_ERROR("Couldn't send message errno: ", errno);
    return false;
  }

  return true;
}

bool Client::receive(char *outbuf) {
  if (m_socketfd == INVALID_SOCKET) {
    LOG_ERROR("Trying to use uninitialized Client");
    return false;
  }
  if (recv(m_socketfd, outbuf, 512, 0) < 0) {
    outbuf[0] = 0;
    LOG_ERROR("Couldn't receive message errno:", errno);
    return false;
  }
  return true;
}

} // namespace network

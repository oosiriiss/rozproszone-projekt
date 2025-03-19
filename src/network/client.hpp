#pragma once

namespace network {
class Client {

public:
  Client();
  ~Client();

  bool connect(const char *ipAddress, unsigned short port);
  bool send(const char *msg);
  bool receive(char *outBuf);

private:
  // client scoket description
  int m_socketfd;
};
}; // namespace network

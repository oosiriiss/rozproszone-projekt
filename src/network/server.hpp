#pragma once
#include <netinet/in.h>
#include <vector>

namespace network {

struct ClientData {
  int socketfd;
  // TODO :: create my own wrapper?
  struct sockaddr_in connectionData;
};

class Server {

public:
  Server();
  ~Server();

  /**
   * Initializes the server socket.
   * !! Must be called before trying to use any other function !!
   **/
  bool bind(const char *ipAddress, unsigned short port);
  // Accepts clients and adds it to internal client list
  bool acceptClient();
  bool send(const char *msg);
  bool receive(char *outBuf);

private:
  int m_socketfd;

  std::vector<ClientData> m_clients;
};
} // namespace network

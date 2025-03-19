#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "network/client.hpp"
#include "network/server.hpp"

const int PORT = 39213;

int main(int argc, char *argv[]) {

  if (argc > 1 && (strcmp(argv[1], "server") == 0)) {
    network::Server server;

    server.bind("127.0.0.1", PORT);
    server.acceptClient();

    int i = 0;
    while (1) {
      char buf[512] = {0};
      server.receive((char *)&buf);
      std::cout << "Message from client is: " << buf << '\n';
      server.send("this is message from server");

      sleep(2);
    }
  }
  // CLIENT
  else {

    network::Client client;
    client.connect("127.0.0.1", PORT);

    while (true) {
      client.send("Wiadomosc do serwera od klienta");

      char buf[512] = {0};
      client.receive((char *)&buf);

      std::cout << "Message from server is: " << buf << '\n';

      sleep(2);
    }
  }
}

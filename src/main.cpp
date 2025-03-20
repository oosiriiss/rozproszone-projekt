#include <arpa/inet.h>
#include <csignal>
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

sig_atomic_t SIGINT_RECEIVED = 0;

void SIGINT_handler(int s) {
  std::cout << "SIGINT received - closing...\n";
  SIGINT_RECEIVED = 1;
}

int main(int argc, char *argv[]) {

  // SIGKILL HANDLERS
  // to allow graceful shutdown when uses presses ctrl-c
  std::signal(SIGINT, SIGINT_handler);

  if (argc > 1 && (strcmp(argv[1], "server") == 0)) {
    network::Server server;

    server.bind("127.0.0.1", PORT);
    server.waitForClients(1);

    int i = 0;
    while (1) {
      if (SIGINT_RECEIVED)
        break;

      std::string msg = server.receive();
      std::cout << "Message from client is: " << msg << '\n';
      server.send("this is message from serverSEPARATOR");

      usleep(500 * 1000);
    }
  }
  // CLIENT
  else {

    network::Client client;
    client.connect("127.0.0.1", PORT);

    while (true) {
      if (SIGINT_RECEIVED)
        break;

      client.send("Wiadomosc do serwera od klientaSEPARATOR");

      std::string msg = client.receive();
      std::cout << "Message from server is: (" << msg << ")\n";

      usleep(500 * 1000);
    }
  }
}

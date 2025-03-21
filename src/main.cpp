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

  if (argc > 1 && argv[1][0] == 's') {
    network::Server server;

    server.bind("127.0.0.1", PORT);
    server.waitForClients(2);

    std::string msg;

    while (1) {
      if (SIGINT_RECEIVED)
        break;

      // Should be in pollMessage?
      server.receive();

      while (server.pollMessage(msg, "SEPARATOR"))
        std::cout << "Message from client is: " << msg << '\n';

      server.send("this is message from serverSEPARATOR");

      usleep(50 * 1000);
    }
  }
  // CLIENT
  else {

    const char *id = "NOID";

    if (argc > 2)
      id = argv[2];

    std::string send_msg = "wiadomosc od klienta ";
    send_msg.append(id);
    send_msg.append("SEPARATOR");

    network::Client client;
    client.connect("127.0.0.1", PORT);

    std::string message;

    while (true) {
      if (SIGINT_RECEIVED)
        break;

      client.send(send_msg.c_str());

      client.receive();

      if (client.getMessage(message, "SEPARATOR")) {
        std::cout << "Message from server is: (" << message << ")\n";
      }

      usleep(200 * 1000);
    }
  }
}

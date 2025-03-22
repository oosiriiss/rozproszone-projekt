#include "Application.hpp"
#include <arpa/inet.h>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void client(const char *id) {}

int main(int argc, char *argv[]) {

  Application app(argc, argv);

  if (argc > 1 && argv[1][0] == 's') {
    app.server();
  }
  // CLIENT
  else {
    const char *id = "NOID";
    if (argc > 2)
      id = argv[2];
    app.client(id);
  }
}

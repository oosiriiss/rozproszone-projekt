#include "Application.hpp"

void client(const char *id) {}

int main(int argc, char *argv[]) {

  Application app(argc, argv);

  if (argc > 1 && argv[1][0] == 's') {
    app.server();
  }
  // CLIENT
  else {
    app.client();
  }
}

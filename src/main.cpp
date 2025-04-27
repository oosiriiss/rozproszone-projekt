#include "Application.hpp"

int main(int argc, char *argv[]) {

  Application app(argc, argv);
  app.run(argc > 1 && argv[1][0] == 's');
}

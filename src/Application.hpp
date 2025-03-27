#pragma once
#include <SDL3/SDL.h>
#include <SFML/Graphics/RenderWindow.hpp>

class Application {

public:
  Application(int argc, char *argv[]);
  ~Application() = default;

  void server();
  void client();

private:
  sf::RenderWindow m_window;
};

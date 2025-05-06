#pragma once
#include "AssetManager.hpp"
#include "Scene.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Mouse.hpp>

class Application {

public:
  Application(int argc, char *argv[]);
  ~Application() = default;

  void handleEvents();
  void run(bool isServer);

  // Returns the position of the mouse of the last click
  static sf::Vector2f getMousePosition();
  static bool isMousePressed(sf::Mouse::Button b);
  static bool isKeyPressed(sf::Keyboard::Key k);

private:
  SceneManager m_sceneManager;
  sf::RenderWindow m_window;
  AssetManager m_assetManager;

  static sf::Vector2i s_mousePos;
  static std::vector<sf::Mouse::Button> s_mouseButtons;
  static std::vector<sf::Keyboard::Key> s_pressedKeys;
};

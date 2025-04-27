#pragma once
#include "AssetManager.hpp"
#include "Scene.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class Application {

public:
  Application(int argc, char *argv[]);
  ~Application() = default;

  void handleEvents();
  void run(bool isServer);

  static bool isKeyPressed(sf::Keyboard::Key k);

private:
  SceneManager m_sceneManager;
  sf::RenderWindow m_window;
  AssetManager m_assetManager;

  static std::vector<sf::Keyboard::Key> s_pressedKeys;
};

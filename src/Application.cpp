#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <csignal>
#include <iostream>

#include "Application.hpp"
#include "ui/ui.hpp"

sf::Vector2i Application::s_mousePos = {-1, -1};

std::vector<sf::Mouse::Button> Application::s_mouseButtons =
    std::vector<sf::Mouse::Button>();

std::vector<sf::Keyboard::Key> Application::s_pressedKeys =
    std::vector<sf::Keyboard::Key>();

sig_atomic_t SIGINT_RECEIVED = 0;
void SIGINT_handler(int s) {
  std::cout << "SIGINT received - closing...\n";
  SIGINT_RECEIVED = 1;
}

Application::Application(int argc, char *argv[]) : m_assetManager("./assets") {

  const char *title = "Client";

  if (argc > 1 && argv[1][0] == 's') {
    title = "Server";
  } 
  constexpr int WINDOW_WIDTH = 640;
  constexpr int WINDOW_HEIGHT = 640;
  constexpr int WINDOW_STYLE = sf::Style::Titlebar;

  m_window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
                              title, WINDOW_STYLE);

  // Centering the window
  auto desktop = sf::VideoMode::getDesktopMode().size;
  m_window.setPosition(
      {static_cast<int>(desktop.x / 2 - m_window.getSize().x / 2),
       static_cast<int>(desktop.y / 2 - m_window.getSize().y / 2)});

  m_window.setFramerateLimit(60);
  // SIGKILL HANDLER
  // to allow graceful shutdown when uses presses ctrl-c
  std::signal(SIGINT, SIGINT_handler);

  //
  // Loading assets
  //

  m_assetManager.loadFont("font.ttf");

  ui::g_UIContext.init(&m_window, &m_assetManager);
}

void Application::run(bool isServer) {

  sf::Clock deltaTimer;

  const char *IP = "127.0.0.1";
  uint16_t port = 63921;

  if (isServer)
    m_sceneManager.pushScene(
        new ConnectServerScene(IP, port, m_sceneManager, m_window));
  else
    m_sceneManager.pushScene(
        new ConnectClientScene(IP, port, m_sceneManager, m_window));

  while (m_window.isOpen()) {
    float dt = deltaTimer.restart().asSeconds();

    if (SIGINT_RECEIVED)
      break;

    handleEvents();

    m_sceneManager.getCurrentScene()->update(dt);
    m_window.clear();
    m_sceneManager.getCurrentScene()->draw();
    m_window.display();

    ui::g_UIContext.endDraw();
  }
}

void Application::handleEvents() {
  s_pressedKeys.clear();
  s_mouseButtons.clear();

  while (const std::optional event = m_window.pollEvent()) {
    if (event->is<sf::Event::Closed>())
      m_window.close();
    if (auto *mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
      s_mouseButtons.push_back(mouse->button);
      s_mousePos = mouse->position;

      ui::g_UIContext.mousePos = {mouse->position.x, mouse->position.y};
      ui::g_UIContext.didClickMouse = mouse->button == sf::Mouse::Button::Left;
    } else if (auto *keyboard = event->getIf<sf::Event::KeyPressed>()) {
      s_pressedKeys.push_back(keyboard->code);
    }
  }
}

sf::Vector2f Application::getMousePosition() {
  return {static_cast<float>(s_mousePos.x), static_cast<float>(s_mousePos.y)};
}

bool Application::isKeyPressed(sf::Keyboard::Key k) {
  return std::find(s_pressedKeys.begin(), s_pressedKeys.end(), k) !=
         s_pressedKeys.end();
}

bool Application::isMousePressed(sf::Mouse::Button b) {
  return std::find(s_mouseButtons.begin(), s_mouseButtons.end(), b) !=
         s_mouseButtons.end();
}

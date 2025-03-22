#include "Application.hpp"
#include "logging.hpp"
#include "network/client.hpp"
#include "network/packet.hpp"
#include "network/server.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <arpa/inet.h>
#include <csignal>
#include <iostream>
#include <variant>

const int PORT = 39213;
sig_atomic_t SIGINT_RECEIVED = 0;

void SIGINT_handler(int s) {
  std::cout << "SIGINT received - closing...\n";
  SIGINT_RECEIVED = 1;
}

Application::Application(int argc, char *argv[]) {

  const char *title = "INVALID TITLE";

  if (argc > 1 && argv[1][0] == 's') {
    title = "Server";
  } else if (argc > 2) {
    title = argv[2];
  }

  constexpr int WINDOW_WIDTH = 640;
  constexpr int WINDOW_HEIGHT = 640;
  constexpr int WINDOW_STYLE = sf::Style::Default;

  m_window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
                              title, WINDOW_STYLE);

  m_window.setFramerateLimit(60);
  // SIGKILL HANDLER
  // to allow graceful shutdown when uses presses ctrl-c
  std::signal(SIGINT, SIGINT_handler);
}

void Application::server() {

  network::Server server;

  server.bind("127.0.0.1", PORT);
  LOG_INFO("Awaiting clients");
  server.waitForClients(2);
  LOG_INFO("Clients connected");

  uint8_t ids = 0;

  while (m_window.isOpen()) {
    if (SIGINT_RECEIVED)
      break;

    while (const std::optional event = m_window.pollEvent()) {
      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>())
        m_window.close();
    }

    // Should be in pollMessage?
    server.receive();

    while (auto sockmsg = server.pollMessage(network::SEPARATOR)) {
      LOG_INFO("POLLING");

      auto [socket, msg] = *sockmsg;

      if (auto rcvd = network::decodeClientPacket(msg)) {
        if (auto *request = std::get_if<network::AcquireIDRequest>(&(*rcvd))) {
          std::cout << "Received acquire id request from client: ";

          network::AcquireIDResponse r = {.id = ids++};
          std::string response = network::encodePacket(r);

          server.send(response.c_str(), response.size());
        }
      } else {
        LOG_ERROR("Decoding client packet error\n");
      }
    }

    m_window.display();
  }
}

void Application::client(const char *id) {

  sf::RectangleShape playerRect;
  playerRect.setFillColor(sf::Color(255, 255, 255));
  playerRect.setPosition({300.f, 300.f});
  playerRect.setSize({50.f, 50.f});

  //
  //

  network::AcquireIDRequest r = {};
  auto msg = network::encodePacket(r);

  network::Client client;
  client.connect("127.0.0.1", PORT);

  while (m_window.isOpen()) {
    if (SIGINT_RECEIVED)
      break;

    while (const std::optional event = m_window.pollEvent()) {

      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>())
        m_window.close();
      else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {

        if (keyPressed->code == sf::Keyboard::Key::W) {
          playerRect.move({0, -1.f});
        }
        if (keyPressed->code == sf::Keyboard::Key::S) {
          playerRect.move({0, 1.f});
        }
        if (keyPressed->code == sf::Keyboard::Key::A) {
          playerRect.move({-1.f, 0});
        }
        if (keyPressed->code == sf::Keyboard::Key::D) {
          playerRect.move({1.f, 0});
        }
      }
    }

    LOG_INFO("MSG IS", msg);
    client.send(msg.c_str(), msg.size());
    client.receive();
    while (auto msg = client.pollMessage(network::SEPARATOR)) {
      if (auto rcvd = network::decodeServerPacket(*msg)) {

        if (auto *response =
                std::get_if<network::AcquireIDResponse>(&(*rcvd))) {

          std::cout << "Received acquire id response with id: " << response->id
                    << '\n';
        }

      } else {
        LOG_ERROR("Decoding server packer error");
      }
    }

    m_window.clear();

    m_window.draw(playerRect);

    m_window.display();
  }
}

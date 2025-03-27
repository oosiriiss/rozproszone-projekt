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
#include <unordered_map>
#include <utility>
#include <variant>

#include "Application.hpp"
#include "logging.hpp"
#include "network/client.hpp"
#include "network/packet.hpp"
#include "network/server.hpp"

#include "game/player.hpp"
#include "network/socket.hpp"

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

  std::unordered_map<uint8_t, Player> players;
  network::Server server;

  server.bind("127.0.0.1", PORT);
  LOG_INFO("Awaiting clients");
  server.waitForClients(2);
  LOG_INFO("Clients connected");

  uint8_t ids = 1;

  while (m_window.isOpen()) {
    if (SIGINT_RECEIVED)
      break;

    while (const std::optional event = m_window.pollEvent()) {
      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>())
        m_window.close();
    }

    // Should be in pollMessage?
    auto receiveError = server.receive();
    if (receiveError) {
      LOG_ERROR("Couldn't receive from a client error: ",
                std::to_underlying(*receiveError));
    }

    while (auto sockmsg = server.pollMessage(network::SEPARATOR)) {
      LOG_INFO("POLLING MSGS");
      auto [socket, msg] = *sockmsg;

      if (auto rcvd = network::decodeClientPacket(msg)) {
        if (auto *request = std::get_if<network::AcquireIDRequest>(&(*rcvd))) {
          std::cout << "Received acquire id request from client: ";

          network::AcquireIDResponse r = {.id = ids++};
          LOG_INFO("sent id ", (int)r.id);
          std::string response = network::encodePacket(r);

          socket->send(response.c_str(), response.size());
        } else if (auto *moved =
                       std::get_if<network::PlayerMovedRequest>(&(*rcvd))) {
          LOG_INFO("Player ", moved->playerId, " To position x: ", moved->x,
                   " y: ", moved->y);
          // Moving player on the server
          if (!players.contains(moved->playerId)) {
            players[moved->playerId] = Player(moved->playerId);
            Player &other = players[moved->playerId];
            other.rect.setSize({50, 50});
            other.rect.setFillColor(sf::Color(255, 0, 0));
          }
          Player &other = players[moved->playerId];
          other.rect.setPosition({moved->x, moved->y});

          // client packet to server

          std::string m = network::encodePacket(network::PlayerMovedResponse{
              .playerId = moved->playerId, .x = moved->x, .y = moved->y});

          for (int i = 0; i < server.m_clients.size(); ++i) {
            // player indexes starts from 1
            if (i + 1 == moved->playerId)
              continue;
            server.m_clients[i].send(m.data(), m.size());
          }

          // Sending that data to other players
        }
      } else {
        LOG_ERROR("Decoding client packet error\n");
      }
    }

    m_window.clear();

    for (const auto &[id, plr] : players) {
      m_window.draw(plr.rect);
    }

    m_window.display();
  }
}

void Application::client() {
  uint8_t playerId = 255;
  Player p;
  std::unordered_map<uint8_t, Player> others;

  p.rect.setFillColor(sf::Color(255, 255, 255));
  p.rect.setPosition({300.f, 300.f});
  p.rect.setSize({50.f, 50.f});

  network::AcquireIDRequest r = {};
  auto msg = network::encodePacket(r);

  network::Client client;
  client.connect("127.0.0.1", PORT);

  bool x = true;

  while (m_window.isOpen()) {
    if (SIGINT_RECEIVED)
      break;

    while (const std::optional event = m_window.pollEvent()) {

      // Player has not received an id yet
      if (playerId == 255 && x) {
        network::AcquireIDRequest r = {};
        auto msg = network::encodePacket(r);
        client.send(msg.c_str(), msg.size());
        x = false;
      }

      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>())
        m_window.close();
      else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::W) {
          p.rect.move({0, -1.f});
        }
        if (keyPressed->code == sf::Keyboard::Key::S) {
          p.rect.move({0, 1.f});
        }
        if (keyPressed->code == sf::Keyboard::Key::A) {
          p.rect.move({-1.f, 0});
        }
        if (keyPressed->code == sf::Keyboard::Key::D) {
          p.rect.move({1.f, 0});
        }
        LOG_INFO("player moved");
        network::PlayerMovedRequest pack = {.playerId = playerId,
                                            .x = p.rect.getPosition().x,
                                            .y = p.rect.getPosition().y};

        auto msg = network::encodePacket(pack);

        client.send(msg.data(), msg.size());
      }
    }

    client.receive();
    while (auto msg = client.pollMessage(network::SEPARATOR)) {
      if (auto rcvd = network::decodeServerPacket(*msg)) {
        if (auto *response =
                std::get_if<network::AcquireIDResponse>(&(*rcvd))) {

          std::cout << "Received acquire id response with id: " << response->id
                    << '\n';
          playerId = response->id;
        }
        if (auto *response =
                std::get_if<network::PlayerMovedResponse>(&(*rcvd))) {
          if (others.contains(response->playerId)) {
            LOG_INFO("RECEIVED MOVE RESPONSE");
            others[response->playerId] = Player(response->playerId);
            Player &other = others[response->playerId];
            other.rect.setSize({50, 50});
            other.rect.setFillColor(sf::Color(255, 0, 0));
          }
          Player &other = others[response->playerId];
          other.rect.setPosition({response->x, response->y});
        }
      } else {
        LOG_ERROR("Decoding server packer error");
      }
    }

    m_window.clear();

    m_window.draw(p.rect);

    for (const auto &[id, plr] : others) {
      m_window.draw(plr.rect);
    }

    m_window.display();
  }
}

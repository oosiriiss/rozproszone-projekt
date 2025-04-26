#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <arpa/inet.h>
#include <csignal>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <variant>

#include "Application.hpp"
#include "Scene.hpp"
#include "game/Player.hpp"
#include "logging.hpp"
#include "network/packet.hpp"
#include "network/socket.hpp"
#include "ui/ui.hpp"

sig_atomic_t SIGINT_RECEIVED = 0;

void SIGINT_handler(int s) {
  std::cout << "SIGINT received - closing...\n";
  SIGINT_RECEIVED = 1;
}

Application::Application(int argc, char *argv[]) : m_assetManager("./assets") {

  const char *title = "INVALID TITLE";

  if (argc > 1 && argv[1][0] == 's') {
    title = "Server";
  } else if (argc > 2) {
    title = argv[2];
  }

  constexpr int WINDOW_WIDTH = 640;
  constexpr int WINDOW_HEIGHT = 640;
  constexpr int WINDOW_STYLE = sf::Style::None;

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

  const char *IP = "127.0.0.1";
  uint16_t port = 63921;

  if (isServer)
    m_sceneManager.pushScene(new ConnectServerScene(IP, port, m_sceneManager));
  else
    m_sceneManager.pushScene(new ConnectClientScene(IP, port, m_sceneManager));

  while (m_window.isOpen()) {
    if (SIGINT_RECEIVED)
      break;

    handleEvents();

    m_sceneManager.getCurrentScene()->update();

    m_window.clear();
    m_sceneManager.getCurrentScene()->draw();
    m_window.display();

    ui::g_UIContext.endDraw();
  }
}

void Application::handleEvents() {
  while (const std::optional event = m_window.pollEvent()) {
    // "close requested" event: we close the window
    if (event->is<sf::Event::Closed>())
      m_window.close();
    if (auto *mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
      ui::g_UIContext.mousePos = {mouse->position.x, mouse->position.y};

      ui::g_UIContext.didClickMouse = mouse->button == sf::Mouse::Button::Left;
    }
  }
}

// void Application::server() {
//   std::unordered_map<uint8_t, Player> players;
//   network::Server server;
//
//   server.bind("127.0.0.1", PORT);
//   server.waitForClients(2);
//
//   while (m_window.isOpen()) {
//
//     // Should be in pollMessage?
//     auto receiveError = server.receive();
//     if (receiveError) {
//       LOG_ERROR("Couldn't receive from a client error: ",
//                 std::to_underlying(*receiveError));
//     }
//
//     while (auto sockmsg = server.pollMessage()) {
//
//       LOG_INFO("POLLING MSGS");
//       auto [socket, msg] = *sockmsg;
//
//       if (auto rcvd = network::decodeClientPacket(msg)) {
//         handleClientPacket(&(*(rcvd)), server, *socket, players);
//       } else {
//         LOG_ERROR("Decoding client packet error\n");
//       }
//     }
//
//     m_window.clear();
//     for (const auto &[id, plr] : players) {
//       m_window.draw(plr.rect);
//     }
//
//     m_window.display();
//   }
// }

// void Application::client() {
//   if (ui::Button("Hello")) {
//     std::cout << "Button clicked\n";
//   } else {
//     std::cout << "Not clicked\n";
//   }
//
//   Player p;
//   std::unordered_map<uint8_t, Player> others;
//
//   p.rect.setFillColor(sf::Color(255, 255, 255));
//   p.rect.setPosition({300.f, 300.f});
//   p.rect.setSize({50.f, 50.f});
//
//   network::AcquireIDRequest r = {};
//   auto msg = network::encodePacket(r);
//
//   network::Client client;
//   client.connect("127.0.0.1", PORT);
//
//   while (m_window.isOpen()) {
//     if (SIGINT_RECEIVED)
//       break;
//
//     while (const std::optional event = m_window.pollEvent()) {
//
//       // Player has not received an id yet
//       if (p.id == 255) {
//         network::AcquireIDRequest r = {};
//         auto msg = network::encodePacket(r);
//         client.send(msg.c_str(), msg.size());
//       }
//
//       // "close requested" event: we close the window
//       if (event->is<sf::Event::Closed>())
//         m_window.close();
//       else if (const auto *keyPressed =
//       event->getIf<sf::Event::KeyPressed>()) {
//         p.handleKeydown(keyPressed->code);
//
//         LOG_INFO("player moved");
//
//         network::PlayerMovedRequest pack = {.playerId = p.id,
//                                             .x = p.rect.getPosition().x,
//                                             .y = p.rect.getPosition().y};
//
//         auto msg = network::encodePacket(pack);
//         client.send(msg.data(), msg.size());
//       }
//     }
//     client.receive();
//     while (auto msg = client.pollMessage()) {
//       if (auto rcvd = network::decodeServerPacket(*msg)) {
//       } else {
//         LOG_ERROR("Decoding server packer error");
//       }
//     }
//
//     m_window.clear();
//
//     p.draw(m_window);
//
//     for (const auto &[id, plr] : others) {
//       plr.draw(m_window);
//     }
//
//     m_window.display();
//   }
// }

// void Application::handleClientPacket(
//     const network::ClientPacket *packet, network::Server &server,
//     network::Socket &client, std::unordered_map<uint8_t, Player> &players) {
//
//   static uint8_t ids = 1;
//
//   if (auto *request = std::get_if<network::AcquireIDRequest>(packet)) {
//     std::cout << "Received acquire id request from client: ";
//
//     network::AcquireIDResponse r = {.id = ids++};
//     LOG_INFO("sent id ", (int)r.id);
//     std::string response = network::encodePacket(r);
//     client.send(response.c_str(), response.size());
//   } else if (auto *moved = std::get_if<network::PlayerMovedRequest>(packet))
//   {
//     LOG_INFO("Player ", moved->playerId, " To position x: ", moved->x,
//              " y: ", moved->y);
//     // Moving player on the server
//     if (!players.contains(moved->playerId)) {
//       players[moved->playerId] = Player(moved->playerId);
//       Player &other = players[moved->playerId];
//       other.rect.setSize({50, 50});
//       other.rect.setFillColor(sf::Color(255, 0, 0));
//     }
//     Player &other = players[moved->playerId];
//     other.rect.setPosition({moved->x, moved->y});
//
//     // client packet to server
//
//     std::string m = network::encodePacket(network::PlayerMovedResponse{
//         .playerId = moved->playerId, .x = moved->x, .y = moved->y});
//
//     for (int i = 0; i < server.m_clients.size(); ++i) {
//       // player indexes starts from 1
//       if (i + 1 == moved->playerId)
//         continue;
//       server.m_clients[i].send(m.data(), m.size());
//     }
//     // Sending that data to other players
//   }
// }
//
// void Application::handleServerPacket(
//     const network::ServerPacket *packet, network::Client &client,
//     Player &player, std::unordered_map<uint8_t, Player> &others) {
//
//   if (auto *response = std::get_if<network::AcquireIDResponse>(packet)) {
//
//     std::cout << "Received acquire id response with id: " << response->id
//               << '\n';
//     player.id = response->id;
//   }
//   if (auto *response = std::get_if<network::PlayerMovedResponse>(packet)) {
//     if (others.contains(response->playerId)) {
//       LOG_INFO("RECEIVED MOVE RESPONSE");
//       others[response->playerId] = Player(response->playerId);
//       Player &other = others[response->playerId];
//       other.rect.setSize({50, 50});
//       other.rect.setFillColor(sf::Color(255, 0, 0));
//     }
//     Player &other = others[response->playerId];
//     other.rect.setPosition({response->x, response->y});
//   }
// }

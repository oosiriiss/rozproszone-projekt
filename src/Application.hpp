#pragma once
#include "AssetManager.hpp"
#include "Scene.hpp"
#include "game/Player.hpp"
#include "network/client.hpp"
#include "network/packet.hpp"
#include "network/server.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class Application {

public:
  Application(int argc, char *argv[]);
  ~Application() = default;

  void handleEvents();

  void server();
  void client();
  void run(bool isServer);

  static void handleClientPacket(const network::ClientPacket *packet,
                                 network::Server &server,
                                 network::Socket &client,
                                 std::unordered_map<uint8_t, Player> &players);

  static void handleServerPacket(const network::ServerPacket *packet,
                                 network::Client &client, Player &player,
                                 std::unordered_map<uint8_t, Player> &others);

private:
  SceneManager m_sceneManager;
  sf::RenderWindow m_window;
  AssetManager m_assetManager;
};

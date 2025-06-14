#pragma once

#include "game/Player.hpp"
#include "network/client.hpp"
#include "network/server.hpp"
#include <memory>
#include <stack>
#include <unordered_map>

class SceneManager;

#define SCENE_PARAMS SceneManager &sceneManager, sf::RenderWindow &window
#define SCENE_ARGS m_sceneManager, m_window
#define SCENE_CONSTRUCTOR Scene(sceneManager, window)

struct Scene {

  Scene(SCENE_PARAMS);
  virtual ~Scene() = default;

  virtual void update(float dt) = 0;
  virtual void resume() {};
  virtual void draw() = 0;

protected:
  SceneManager &m_sceneManager;
  sf::RenderWindow &m_window;
};

class SceneManager {
public:
  SceneManager() = default;
  ~SceneManager();

  // Scene::init is automaticcaly called whenever scene is pushed onto the
  // stack
  void pushScene(Scene *scene);
  // Scene::destroy is automaticcaly called whenever scene is pushed onto the
  // stack
  void popScene();
  Scene *getCurrentScene() const;

private:
  std::stack<Scene *> m_scenes;
};
class ConnectClientScene : public Scene {

public:
  ConnectClientScene(const char *ip, uint16_t port, SCENE_PARAMS);
  ~ConnectClientScene();

  void update(float dt) override;
  void resume() override;
  void draw() override;

  const char *targetIP;
  const uint16_t targetPort;

private:
  std::shared_ptr<network::Client> m_client;
  bool m_isConnected = false;
  bool m_isReady = false;

  std::unordered_map<int32_t, bool> m_lobbyMembers;

  SceneManager &m_sceneManager;
  sf::RenderWindow &m_window;
};

class ConnectServerScene : public Scene {

public:
  ConnectServerScene(const char *ip, uint16_t port, SCENE_PARAMS);
  ~ConnectServerScene();

  void update(float dt) override;
  void resume() override;
  void draw() override;

  bool allPlayersReady() const;

  const char *bindIP;
  const uint16_t bindPort;

private:
  std::shared_ptr<network::Server> m_server;
  bool m_isBound = false;

  std::unordered_map<int32_t, bool> m_lobbyMembers;

private:
};

class ClientGameScene : public Scene {
public:
  ClientGameScene(std::shared_ptr<network::Client> client, SCENE_PARAMS);
  ~ClientGameScene();

  void update(float dt) override;
  void draw() override;

private:
  std::shared_ptr<network::Client> m_client;
  Level m_level;
  Player m_player;
  std::unordered_map<uint8_t, Player> m_otherPlayers;

  bool m_isInitialized = false;

  float m_playerSyncTimer = 0.f;
  float FULL_SYNC_THRESHOLD = 1.f;
};

class ServerGameScene : public Scene {
public:
  ServerGameScene(std::shared_ptr<network::Server> server, SCENE_PARAMS);
  ~ServerGameScene();

  void update(float dt) override;
  void draw() override;

private:
  std::shared_ptr<network::Server> m_server;
  Level m_level;

  std::unordered_map<int32_t, Player> m_players;

  float m_fullSyncTimer = 0.f;
};

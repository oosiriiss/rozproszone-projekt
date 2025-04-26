#pragma once

#include "logging.hpp"
#include "network/client.hpp"
#include "network/server.hpp"
#include <memory>
#include <stack>

struct Scene {

  Scene() = default;
  virtual ~Scene() = default;

  virtual void update() { LOG_DEBUG("DEFAULT SCENE UPDATE CALLED"); }
  virtual void draw() { LOG_DEBUG("DEFAULT SCENE DRAW CALLED"); };
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
  ConnectClientScene(const char *ip, uint16_t port, SceneManager &sceneManager);
  ~ConnectClientScene();

  void update() override;
  void draw() override;

  const char *targetIP;
  const uint16_t targetPort;

private:
  std::shared_ptr<network::Client> m_client;
  SceneManager &m_sceneManager;
  bool m_isConnected = false;
  uint8_t m_connectedPlayerCount = 0;
};

class ConnectServerScene : public Scene {

public:
  ConnectServerScene(const char *ip, uint16_t port, SceneManager &sceneManager);
  ~ConnectServerScene();

  void update() override;
  void draw() override;

  const char *bindIP;
  const uint16_t bindPort;

private:
  std::shared_ptr<network::Server> m_server;
  SceneManager &m_sceneManager;
  bool m_isBound = false;

  uint8_t m_connectedPlayerCount = 0;

private:
};

class ClientGameScene : public Scene {
public:
  ClientGameScene(std::shared_ptr<network::Client> client);
  ~ClientGameScene();

  void update() override;
  void draw() override;

private:
  network::Client m_client;
};

class ServerGameScene : public Scene {
public:
  ServerGameScene(std::shared_ptr<network::Server> server);
  ~ServerGameScene();

  void update() override;
  void draw() override;

private:
  network::Server server;
};

#include "Scene.hpp"
#include "debug.hpp"
#include "logging.hpp"
#include "network/packet.hpp"
#include "ui/ui.hpp"
#include <memory>

SceneManager::~SceneManager() {
  while (!m_scenes.empty()) {
    delete m_scenes.top();
    m_scenes.pop();
  }
}

void SceneManager::pushScene(Scene *scene) {
  LOG_DEBUG("Adding Scene");
  m_scenes.push(scene);
  LOG_DEBUG("Scene Added");
}

void SceneManager::popScene() {
  ASSERT(!m_scenes.empty());
  delete m_scenes.top();
  m_scenes.pop();
}

Scene *SceneManager::getCurrentScene() const {
  ASSERT(!m_scenes.empty());
  return m_scenes.top();
}

ConnectClientScene::ConnectClientScene(const char *ip, uint16_t port,
                                       SceneManager &sceneManager)
    : targetIP(ip), targetPort(port), m_sceneManager(sceneManager),
      m_client(std::make_shared<network::Client>()) {}
ConnectClientScene::~ConnectClientScene() {}

void ConnectClientScene::update() {

  if (m_isConnected) {
    while (auto msg = m_client->pollMessage()) {
      auto packet = *msg;

      if (auto *jlr = std::get_if<network::JoinLobbyResponse>(&packet)) {
        m_connectedPlayerCount = jlr->connectedPlayersCount;
      } else {
        LOG_ERROR("Unknown packet");
      }
    }
  }
}
void ConnectClientScene::draw() {
  ui::Text(" ");
  ui::Text("Server Address:");
  ui::Text(targetIP);

  if (m_isConnected) {
    ui::Text("Waiting for other players. Connected: " +
             std::to_string(m_connectedPlayerCount));

  } else {
    if (ui::Button("Connect")) {
      LOG_INFO("Connecting to server ", targetIP, ":", targetPort);
      if (m_client->connect(targetIP, targetPort)) {
        LOG_INFO("Client connected");
        m_client->send(network::JoinLobbyRequest{});
        m_isConnected = true;

      } else
        LOG_ERROR("Client couldn't connct");
    }
  }
}

ConnectServerScene::ConnectServerScene(const char *ip, uint16_t port,
                                       SceneManager &sceneManager)
    : bindIP(ip), bindPort(port), m_sceneManager(sceneManager),
      m_server(std::make_shared<network::Server>()) {}
ConnectServerScene::~ConnectServerScene() {}
void ConnectServerScene::update() {

  if (!m_isBound)
    return;

  if (m_server->tryAcceptClient()) {
    LOG_INFO("Client connected");
  }

  while (auto sockmsg = m_server->pollMessage()) {
    auto [socket, msg] = *sockmsg;
    if (auto *jlr = std::get_if<network::JoinLobbyRequest>(&msg)) {
      LOG_INFO("Received JoinLobbyRequest");
      m_connectedPlayerCount++;
      m_server->sendAll(network::JoinLobbyResponse{.connectedPlayersCount =
                                                       m_connectedPlayerCount});
    } else {
      LOG_ERROR("Unknown packet");
    }
  }

  // if (m_connectedPlayerCount == 2) {
  //   m_server->sendAll();
  //   m_sceneManager.pushScene(new ServerGameScene(m_server));
  // }
}
void ConnectServerScene::draw() {
  ui::Text(" ");
  ui::Text("Server Address:" + std::string(bindIP));

  if (m_isBound) {
    ui::Text("Waiting for clients... (Connected: " +
             std::to_string(m_connectedPlayerCount) + ")");
  } else {
    if (ui::Button("Bind")) {
      LOG_INFO("Binding server ", bindIP, ":", bindPort);

      if (m_server->bind(bindIP, bindPort)) {
        LOG_INFO("Server bind success");
        m_isBound = true;
      } else {
        LOG_ERROR("Couldn't bind server");
      }
    }
  }
}

ClientGameScene::ClientGameScene(std::shared_ptr<network::Client> client)
    : m_client(client) {}
ClientGameScene::~ClientGameScene() {}

void ClientGameScene::update() {}
void ClientGameScene::draw() {}

ServerGameScene::ServerGameScene(std::shared_ptr<network::Server> server)
    : m_server(server) {}
ServerGameScene::~ServerGameScene() {}

void ServerGameScene::update() {}
void ServerGameScene::draw() {}

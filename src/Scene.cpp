#include "Scene.hpp"
#include "debug.hpp"
#include "game/Player.hpp"
#include "logging.hpp"
#include "network/packet.hpp"
#include "ui/ui.hpp"
#include <memory>

Scene::Scene(SCENE_PARAMS) : m_sceneManager(sceneManager), m_window(window) {}

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
                                       SCENE_PARAMS)
    : SCENE_CONSTRUCTOR, targetIP(ip), targetPort(port),
      m_sceneManager(sceneManager), m_window(window),
      m_client(std::make_shared<network::Client>()) {}
ConnectClientScene::~ConnectClientScene() {}

void ConnectClientScene::update(float dt) {

  if (m_isConnected) {
    while (auto msg = m_client->pollMessage()) {
      auto packet = *msg;

      if (auto *jlr = std::get_if<network::JoinLobbyResponse>(&packet)) {
        m_connectedPlayerCount = jlr->connectedPlayersCount;
      } else if (auto *sgr = std::get_if<network::StartGameResponse>(&packet)) {
        m_sceneManager.pushScene(new ClientGameScene(m_client, SCENE_ARGS));
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
                                       SCENE_PARAMS)
    : SCENE_CONSTRUCTOR, bindIP(ip), bindPort(port),
      m_server(std::make_shared<network::Server>()) {}
ConnectServerScene::~ConnectServerScene() {}
void ConnectServerScene::update(float dt) {

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

  if (m_connectedPlayerCount == 2) {
    LOG_INFO("Players connected. Starting game");

    auto gamescene = new ServerGameScene(m_server, m_sceneManager, m_window);
    LOG_DEBUG("Game scene created");
    m_sceneManager.pushScene(gamescene);
  }
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

ClientGameScene::ClientGameScene(std::shared_ptr<network::Client> client,
                                 SCENE_PARAMS)
    : SCENE_CONSTRUCTOR, m_client(client), m_level() {

  LOG_INFO("Client game scene");
  m_client->send(network::GameReadyRequest{});
}
ClientGameScene::~ClientGameScene() {}

void ClientGameScene::update(float dt) {
  m_playerSyncTimer += dt;

  while (auto msg = m_client->pollMessage()) {
    auto packet = *msg;
    if (auto *grr = std::get_if<network::GameReadyResponse>(&packet)) {
      m_level = Level(grr->map);
      m_player = Player(grr->playerID);
      m_player.rect.setPosition(grr->playerPos);
      m_isInitialized = true;
      m_client->send(network::FullPlayerSyncRequest{
          .playerID = m_player.id, .playerPos = m_player.rect.getPosition()});
    } else if (auto *fpsr =
                   std::get_if<network::FullPlayerSyncResponse>(&packet)) {

      if (m_otherPlayers.find(fpsr->playerID) == m_otherPlayers.end()) {
        m_otherPlayers[fpsr->playerID] = Player(fpsr->playerID);
      }

      Player &o = m_otherPlayers.at(fpsr->playerID);
      o.rect.setPosition(fpsr->playerPos);
    }
  }

  m_player.update();
  m_level.update(dt);

  if (m_playerSyncTimer > FULL_SYNC_THRESHOLD) {
    m_playerSyncTimer = 0.f;
    m_client->send(network::FullPlayerSyncRequest{
        .playerID = m_player.id, .playerPos = m_player.rect.getPosition()});
  }
}
void ClientGameScene::draw() {
  if (m_isInitialized) {
    m_level.draw(m_window);
    m_player.draw(m_window);

    for (const auto &e : m_otherPlayers) {
      e.second.draw(m_window);
    }

  } else {
    ui::Text("Waiting for initialization...");
  }
}

ServerGameScene::ServerGameScene(std::shared_ptr<network::Server> server,
                                 SCENE_PARAMS)
    : SCENE_CONSTRUCTOR, m_server(server), m_level() {

  LOG_INFO("Server game scene");
  m_server->sendAll(network::StartGameResponse{});
}
ServerGameScene::~ServerGameScene() {}

void ServerGameScene::update(float dt) {

  while (auto sockmsg = m_server->pollMessage()) {
    auto [socket, packet] = *sockmsg;

    if (auto *grr = std::get_if<network::GameReadyRequest>(&packet)) {
      LOG_INFO("Sending initalization packet");
      Player p = Player(m_currentPlayerID++);
      p.rect.setPosition(m_level.getPlayerStartPos());
      m_players[p.id] = p;
      m_server->send(
          socket, network::GameReadyResponse{.playerID = p.id,
                                             .playerPos = p.rect.getPosition(),
                                             .map = m_level.getMapData()});
      LOG_INFO("Initialization packet sent");
    } else if (auto *fpsr =
                   std::get_if<network::FullPlayerSyncRequest>(&packet)) {

      ASSERT(m_players.find(fpsr->playerID) != m_players.end() &&
             "Player with id not found");

      Player &p = m_players.at(fpsr->playerID);

      p.rect.setPosition(fpsr->playerPos);

      auto e = m_server->sendOthers(
          socket, network::FullPlayerSyncResponse{
                      .playerID = p.id, .playerPos = p.rect.getPosition()});

    } else {
      LOG_ERROR("Unknown packet encountered");
    }
  }

  m_level.update(dt);
}
void ServerGameScene::draw() {
  m_level.draw(m_window);
  for (const auto &p : m_players) {
    p.second.draw(m_window);
  }
}

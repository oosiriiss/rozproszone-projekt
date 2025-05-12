#include "Scene.hpp"
#include "Application.hpp"
#include "debug.hpp"
#include "game/Fireball.hpp"
#include "game/Player.hpp"
#include "logging.hpp"
#include "network/packet.hpp"
#include "ui/ui.hpp"
#include <memory>
#include <string>

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
  LOG_DEBUG("Popping scene (Scenes: ", m_scenes.size(), ")");
  delete m_scenes.top();
  m_scenes.pop();
  LOG_DEBUG("Popped (Scenes: ", m_scenes.size(), ")");
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
        m_lobbyMembers[jlr->playerID] = false;
      } else if (auto *sgr = std::get_if<network::StartGameResponse>(&packet)) {
        m_sceneManager.pushScene(new ClientGameScene(m_client, SCENE_ARGS));
      } else if (auto *lrr =
                     std::get_if<network::LobbyReadyResponse>(&packet)) {
        m_lobbyMembers[lrr->playerID] = lrr->isReady;
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
    ui::Text("Waiting for the game to start");
    for (auto [id, isReady] : m_lobbyMembers) {
      ui::Text("Player: " + std::to_string(id) +
               std::string((isReady) ? " Ready " : " Not ready"));
    }
    if (ui::Button("Ready")) {
      m_isReady = !m_isReady;
      m_client->send(network::LobbyReadyRequst{.isReady = m_isReady});
    }
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
      m_lobbyMembers[socket->fd] = false;
      m_server->sendAll(network::JoinLobbyResponse{.playerID = socket->fd});
    } else if (auto *lrr = std::get_if<network::LobbyReadyRequst>(&msg)) {
      m_lobbyMembers[socket->fd] = lrr->isReady;
      m_server->sendAll(network::LobbyReadyResponse{.playerID = socket->fd,
                                                    .isReady = lrr->isReady});
    } else {
      LOG_ERROR("Unknown packet");
    }
  }
}
void ConnectServerScene::draw() {
  ui::Text(" ");
  ui::Text("Server Address:" + std::string(bindIP));

  if (m_isBound) {
    for (auto [id, isReady] : m_lobbyMembers) {
      ui::Text("Player: " + std::to_string(id) +
               std::string((isReady) ? " Ready " : " Not ready"));
    }

    if (m_lobbyMembers.size() >= 2 && allPlayersReady()) {
      if (ui::Button("Press to start")) {
        LOG_INFO("Players connected. Starting game");
        auto gamescene =
            new ServerGameScene(m_server, m_sceneManager, m_window);
        LOG_DEBUG("Game scene created");
        m_sceneManager.pushScene(gamescene);
      }
    } else {
      ui::Text("Waiting for clients... (Connected: " +
               std::to_string(m_lobbyMembers.size()) + ")");
    }
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

bool ConnectServerScene::allPlayersReady() const {
  for (auto [p, ready] : m_lobbyMembers) {
    if (!ready)
      return false;
  }
  return true;
}

ClientGameScene::ClientGameScene(std::shared_ptr<network::Client> client,
                                 SCENE_PARAMS)
    : SCENE_CONSTRUCTOR, m_client(client), m_level() {

  LOG_INFO("Client game scene");
  m_client->send(network::GameReadyRequest{});
}
ClientGameScene::~ClientGameScene() {}

void ClientGameScene::update(float dt) {
  if (Application::isKeyPressed(sf::Keyboard::Key::W)) {
    m_client->send(network::PlayerMoveRequest{Direction::Up});
  }
  if (Application::isKeyPressed(sf::Keyboard::Key::S)) {
    m_client->send(network::PlayerMoveRequest{Direction::Down});
  }
  if (Application::isKeyPressed(sf::Keyboard::Key::A)) {
    m_client->send(network::PlayerMoveRequest{Direction::Left});
  }
  if (Application::isKeyPressed(sf::Keyboard::Key::D)) {
    m_client->send(network::PlayerMoveRequest{Direction::Right});
  }

  if (Application::isMousePressed(sf::Mouse::Button::Left)) {

    sf::Vector2f dir =
        (Application::getMousePosition() - m_player.rect.getPosition() -
         m_player.rect.getSize() / 2.f)
            .normalized();
    m_client->send(network::FireballShotRequest{
        .playerID = m_player.id,
        .fireball = Fireball::DTO{
            .pos = m_player.rect.getGlobalBounds().getCenter(),
            .direction = dir,
        }});
  }

  m_playerSyncTimer += dt;

  while (auto msg = m_client->pollMessage()) {
    LOG_DEBUG("Received message from server");
    auto packet = *msg;
    LOG_DEBUG("Polled client packet2");

    if (auto *grr = std::get_if<network::GameReadyResponse>(&packet)) {
      LOG_DEBUG("Game ready response");
      m_level = Level(grr->map, false);
      LOG_DEBUG("Level loaded");

      m_player = Player(grr->thisPlayerID);
      m_player.rect.setPosition(grr->thisPlayerPos);

      m_otherPlayers[grr->otherID] = Player(grr->otherID);
      m_otherPlayers[grr->otherID].rect.setPosition(grr->otherPlayerPos);

      LOG_DEBUG("Players loaded");

      m_isInitialized = true;
    } else if (auto *fpsr = std::get_if<network::PlayerMoveResponse>(&packet)) {
      LOG_DEBUG("Player Move response");

      if (fpsr->playerID == m_player.id) {
        m_player.rect.setPosition(fpsr->newPos);

      } else {
        ASSERT(m_otherPlayers.contains(fpsr->playerID));
        m_otherPlayers[fpsr->playerID].rect.setPosition(fpsr->newPos);
      }
    } else if (auto *eur = std::get_if<network::EnemyUpdateResponse>(&packet)) {
      LOG_DEBUG("Upadting enemies");
      m_level.enemies.clear();
      for (auto &enemyDTO : eur->enemies) {
        Enemy e = Enemy(enemyDTO.pos, enemyDTO.destination);
        e.healthBar.health = enemyDTO.health;
        m_level.enemies.push_back(e);
      }
    } else if (auto *ufr =
                   std::get_if<network::UpdateFireballsResponse>(&packet)) {

      m_level.fireballs.clear();
      for (const auto &fireball : ufr->fireballs) {
        m_level.fireballs.push_back(Fireball(fireball.pos, fireball.direction));
      }
    } else if (auto *bhr = std::get_if<network::BaseHitResponse>(&packet)) {
      m_level.base.healthbar.health = bhr->newHealth;
    } else if (auto *gor = std::get_if<network::GameOverResponse>(&packet)) {

      if (gor->isWon)
        LOG_INFO("Game won!");
      else
        LOG_INFO("Game lost.");
      m_sceneManager.popScene();
      return;

    } else {
      LOG_DEBUG("Unknown packet with index: ", packet.index());
      ASSERT(false && "Look debug message above");
    }
  }

  m_player.update();
  m_level.update(dt);
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
    : SCENE_CONSTRUCTOR, m_server(server), m_level(Level::Map1Data, true) {

  LOG_INFO("Server game scene");

  m_server->sendAll(network::StartGameResponse{});

  for (const auto &client : m_server->getClients()) {
    m_players[client.fd] = Player(client.fd);
    m_players[client.fd].rect.setPosition(m_level.getPlayerStartPos());
  }
}

ServerGameScene::~ServerGameScene() {}

void ServerGameScene::update(float dt) {

  while (auto sockmsg = m_server->pollMessage()) {
    auto [socket, packet] = *sockmsg;

    if (auto *grr = std::get_if<network::GameReadyRequest>(&packet)) {
      LOG_INFO("Sending initalization packet");
      const Player &p1 = m_players[socket->fd];

      ASSERT(m_players.size() == 2);

      std::array<Player, 2> plrs;

      int idx = 0;

      Player p2;
      for (auto &p : m_players) {
        if (p.second.id == p1.id)
          continue;
        p2 = p.second;
      }

      m_server->send(socket, network::GameReadyResponse{
                                 .thisPlayerID = p1.id,
                                 .thisPlayerPos = p1.rect.getPosition(),
                                 .otherID = p2.id,
                                 .otherPlayerPos = p2.rect.getPosition(),
                                 .map = m_level.getMapData(),
                             });
      LOG_INFO("Initialization packet sent");
    } else if (auto *pmr = std::get_if<network::PlayerMoveRequest>(&packet)) {

      Player &p = m_players[socket->fd];

      // if (m_level.canMove(p, toVec(pmr->direction))) {
      p.rect.move(toVec(pmr->direction));
      auto e = m_server->sendAll(network::PlayerMoveResponse{
          .playerID = p.id, .newPos = p.rect.getPosition()});
      //}
    } else if (auto *fsr = std::get_if<network::FireballShotRequest>(&packet)) {
      ASSERT(m_players.find(fsr->playerID) != m_players.end());
      m_level.fireballs.push_back(
          Fireball(fsr->fireball.pos, fsr->fireball.direction));
      // m_server->sendAll(network::{
      //     .pos = fsr->pos, .direction = fsr->direction});
    } else {
      LOG_ERROR("Unknown packet encountered with index:", packet.index());
      ASSERT(false && "look debug msg before");
    }
  }

  m_level.update(dt);

  m_level.handleFireballHits();
  if (m_level.handleBaseHits()) {
    if (m_level.base.healthbar.health <= 0)
      m_server->sendAll(network::GameOverResponse{.isWon = false});
    else
      m_server->sendAll(
          network::BaseHitResponse{.newHealth = m_level.base.healthbar.health});
  }

  if (m_level.isLevelFinished()) {
    m_server->sendAll(network::GameOverResponse{.isWon = true});
    m_sceneManager.popScene();
    return;
  }

  // Sending updated enemies to the clients

  m_fullSyncTimer += dt;
  if (m_fullSyncTimer > 0.05f) {
    m_fullSyncTimer = 0;
    std::vector<Enemy::DTO> enemyDTOs;
    enemyDTOs.reserve(m_level.enemies.size());

    for (const auto &enemy : m_level.enemies) {
      enemyDTOs.push_back(Enemy::DTO{.pos = enemy.rect.getPosition(),
                                     .destination = enemy.destination,
                                     .health = enemy.healthBar.health});
    }

    m_server->sendAll(network::EnemyUpdateResponse(enemyDTOs));

    std::vector<Fireball::DTO> fireballDTOs;
    fireballDTOs.reserve(m_level.fireballs.size());

    for (const auto &fireball : m_level.fireballs) {
      fireballDTOs.push_back(Fireball::DTO{
          .pos = fireball.rect.getPosition(),
          .direction = fireball.direction,
      });
    }

    m_server->sendAll(network::UpdateFireballsResponse(fireballDTOs));
  }
}
void ServerGameScene::draw() {
  m_level.draw(m_window);
  for (const auto &p : m_players) {
    p.second.draw(m_window);
  }
}

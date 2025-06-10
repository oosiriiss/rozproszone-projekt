#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <cstring>

#include "../debug.hpp"
#include "../logging.hpp"
#include "Base.hpp"
#include "Level.hpp"

Tile::Tile() : rect(), type(TileType::Count) {}
Tile::Tile(float x, float y, float tileSize, TileType type)
    : rect(), type(type) {

  this->rect.setSize({tileSize, tileSize});
  this->rect.setPosition({x, y});
  this->rect.setFillColor(getColor(type));

  this->update(x, y, type);
}

void Tile::update(float x, float y, TileType type) {
  this->type = type;
  this->rect.setPosition({x, y});
  this->rect.setFillColor(getColor(type));
}

sf::Color Tile::getColor(TileType type) {
  switch (type) {
  case TileType::Ground:
    return sf::Color(88, 57, 39);
  case TileType::Wall:
    return sf::Color(120, 120, 120);
  case TileType::PlayerStart:
    return sf::Color::Green;
  case TileType::EnemySpawner:
    return sf::Color(180, 120, 120);
  case TileType::Base:
    return sf::Color::Blue;
  case TileType::Count:
    UNREACHABLE;
  }
  UNREACHABLE;
}

Level::Level() : tiles{} {

  LOG_INFO("Loading default level data");
  loadLevel(this->Map1Data);
}
Level::Level(const MapData &tilemap, bool isServer)
    : tiles({}), isServer(isServer) {
  loadLevel(tilemap);
}
Level::~Level() { LOG_INFO("Destroying level"); }

void Level::draw(sf::RenderWindow &window) const {

  for (const auto &tile : this->tiles) {
    window.draw(tile.rect);
  }

  for (const auto &e : enemies) {
    e.draw(window);
  }

  for (const auto &f : fireballs) {
    f.draw(window);
  }

  base.draw(window);
}

void Level::loadLevel(const Level::MapData &data) {
  ASSERT(this->tiles.max_size() == data.tiles.max_size());

  this->m_currentMapID = data.id;

  LOG_DEBUG("Loading level: ", (int)data.id);

  sf::Vector2f basePos;

  for (int i = 0; i < tiles.max_size(); ++i) {
    const int col = i % MAP_WIDTH;
    const int row = i / MAP_WIDTH;

    const int x = col * TILE_SIZE;
    const int y = row * TILE_SIZE;
    if (static_cast<TileType>(data.tiles[i]) == TileType::Base)
      basePos = {x + TILE_SIZE / 2.f, y + TILE_SIZE / 2.f};
    this->base = Base({x * 1.f, y * 1.f});
  }

  for (int i = 0; i < tiles.max_size(); ++i) {

    const int col = i % MAP_WIDTH;
    const int row = i / MAP_WIDTH;

    const int x = col * TILE_SIZE;
    const int y = row * TILE_SIZE;

    TileType tile = data.tiles[i];

    this->tiles[i] = Tile(x, y, Level::TILE_SIZE, tile);

    if (tile == TileType::EnemySpawner) {
      if (this->isServer) {
        spawners.push_back(EnemySpawner(2, 3.f, [this, x, y, basePos]() {
          LOG_DEBUG("Spawning enemy at x: ", x, " y: ", y);
          enemies.push_back(Enemy({x * 1.f, y * 1.f}, basePos));
        }));
      }
    } else if (tile == TileType::Base) {
      this->base.rect.setPosition(
          {static_cast<float>(x), static_cast<float>(y)});
      this->base.healthbar.update(this->base.rect.getGlobalBounds());
    }
  }

  LOG_DEBUG("Level loaded");
}

void Level::update(float dt) {

  this->base.update(dt);

  for (auto &s : spawners) {
    s.update(dt);
  }

  for (auto &e : enemies) {
    if (base.rect.getGlobalBounds().findIntersection(
            e.rect.getGlobalBounds())) {
      // enemy not moving
      e.update(0);
    } else {
      e.update(dt);
    }
  }

  for (auto &f : fireballs) {
    f.update(dt);
  }

  // Removing fireball that are out of the map
  fireballs.resize(std::distance(
      fireballs.begin(),
      std::remove_if(fireballs.begin(), fireballs.end(), [](Fireball &f) {
        sf::Vector2f pos = f.rect.getPosition();
        return (pos.x < 0 || pos.x > MAP_WIDTH * TILE_SIZE || pos.y < 0 ||
                pos.y > MAP_WIDTH * TILE_SIZE);
      })));
}

constexpr sf::Vector2u
Level::calculateTileFromPosition(const sf::Vector2f pos) const {
  const int tx = pos.x / Level::TILE_SIZE;
  const int ty = pos.y / Level::TILE_SIZE;

  ASSERT(tx >= 0 && tx < Level::MAP_WIDTH && "X Tile position within map");
  ASSERT(ty >= 0 && ty < Level::MAP_HEIGHT && "Y Tile position within map");

  return {static_cast<unsigned int>(pos.x / Level::TILE_SIZE),
          static_cast<unsigned int>(pos.y / Level::TILE_SIZE)};
}

bool Level::canMove(const Player &player, sf::Vector2f posDelta) const {

  const sf::Vector2f newPos = player.rect.getPosition() + posDelta;
  const sf::Vector2u newTilePos = calculateTileFromPosition(newPos);

  for (int i = -1; i < 2; ++i) {
    for (int j = -1; j < 2; ++j) {
      if (i == 0 && j == 0)
        continue;
      sf::Vector2u tilePos = {newTilePos.x + j, newTilePos.y + i};
      if (tilePos.x < 0 || tilePos.x >= MAP_WIDTH || tilePos.y < 0 ||
          tilePos.y >= MAP_HEIGHT)
        continue;

      const Tile &tile = tiles[tilePos.y * MAP_WIDTH + tilePos.x];

      if (tile.type == TileType::Wall &&
          tile.rect.getGlobalBounds().findIntersection(
              {newPos, player.rect.getSize()})) {
        LOG_DEBUG("Collision found with: ", tilePos.x, ", ", tilePos.y);
        return false;
      }
    }
  }
  return true;
}

bool Level::isLevelFinished() const {

  if (this->enemies.size() > 0)
    return false;

  for (const auto &spawner : this->spawners) {
    if (!spawner.isDoneSpawning())
      return false;
  }

  return true;
}

void Level::handleFireballHits() {

  for (int f = fireballs.size() - 1; f >= 0; --f) {

    for (int e = enemies.size() - 1; e >= 0; --e) {
      if (enemies[e].rect.getGlobalBounds().findIntersection(
              fireballs[f].rect.getGlobalBounds())) {

        enemies[e].healthBar.health -= 10;
        LOG_INFO("Enemy hit. health left: ", enemies[e].healthBar.health);

        if (enemies[e].healthBar.health <= 0)
          enemies.erase(enemies.begin() + e);

        fireballs.erase(fireballs.begin() + f);
        break;
      }
    }
  }
}

bool Level::handleBaseHits() {
  for (const Enemy &enemy : enemies) {
    if (enemy.rect.getGlobalBounds().findIntersection(
            this->base.rect.getGlobalBounds())) {
      base.damage();
      return true;
    }
  }
  return false;
}

constexpr std::array<TileType, Level::MAP_WIDTH * Level::MAP_HEIGHT>
idsToTypes(const std::array<int, Level::MAP_HEIGHT * Level::MAP_HEIGHT> &ids) {
  std::array<TileType, Level::MAP_WIDTH * Level::MAP_HEIGHT> arr{};
  for (std::size_t i = 0; i < ids.size(); ++i) {
    // The static_assert will cause a compile-time error if an id is invalid.
    // static_assert(ids[i] >= 0 && ids[i] < static_cast<int>(TileType::Count),
    //              "Unknown tile id");
    arr[i] = static_cast<TileType>(ids[i]);
  }
  return arr;
}

sf::Vector2f Level::getPlayerStartPos() const {

  int i = 0;

  for (const Tile &t : this->tiles) {

    if (t.type == TileType::PlayerStart)
      return sf::Vector2f{static_cast<float>((i % MAP_WIDTH) * TILE_SIZE),
                          static_cast<float>((i / MAP_WIDTH) * TILE_SIZE)};

    i = i + 1;
  }

  UNREACHABLE;
}

const Level::MapData &Level::getMapData() const {
  LOG_INFO("Getting Map Data");
  if (m_currentMapID == Level::Map1Data.id) {
    return Level::Map1Data;
  }
  UNREACHABLE;
}

// clang-format off
const Level::MapData Level::Map1Data = {
   .id = 0,
   .tiles = idsToTypes({
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
   })
};
// clang-format on

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <cstring>

#include "../debug.hpp"
#include "../logging.hpp"
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
    break;
  case TileType::EnemySpawner:
    return sf::Color(180, 120, 120);
    break;
  case TileType::Count:
    UNREACHABLE;
  }
  UNREACHABLE;
}

Level::Level() : tiles{} {

  LOG_INFO("Loading default level data");
  loadLevel(this->Map1Data);
}
Level::Level(const MapData &tilemap) : tiles({}) { loadLevel(tilemap); }
Level::~Level() { LOG_INFO("Destroying level"); }

void Level::draw(sf::RenderWindow &window) const {
  for (const auto &tile : this->tiles) {
    window.draw(tile.rect);
  }
  for (const auto &e : enemies) {
    e.draw(window);
  }
}

void Level::loadLevel(const Level::MapData &data) {
  ASSERT(this->tiles.max_size() == data.tiles.max_size());

  this->m_currentMapID = data.id;

  LOG_DEBUG("Loading level: ", (int)data.id);

  for (int i = 0; i < tiles.max_size(); ++i) {

    const int col = i % MAP_WIDTH;
    const int row = i / MAP_WIDTH;

    const int x = col * TILE_SIZE;
    const int y = row * TILE_SIZE;

    TileType tile = data.tiles[i];

    this->tiles[i] = Tile(x, y, Level::TILE_SIZE, tile);

    if (tile == TileType::EnemySpawner) {
      spawners.push_back(EnemySpawner(2, 3.f, [this, x, y]() {
        LOG_DEBUG("Spawning enemy at x: ", x, " y: ", y);
        enemies.push_back(Enemy(x, y));
      }));
    }
  }

  LOG_DEBUG("Level loaded");
}

void Level::update(float dt) {

  for (auto &s : spawners) {
    s.update(dt);
  }

  sf::Vector2u destination = {2, 20};
  for (auto &e : enemies) {

    sf::Vector2u currentTile = calculateTileFromPosition(e.rect.getPosition());
    sf::Vector2u nextTile = findPathTo(currentTile, destination);

    if (nextTile == destination) {
      LOG_DEBUG("Destiantion reached");
    } else {
      sf::Vector2f direction = sf::Vector2f{
          static_cast<float>(nextTile.x) - static_cast<float>(currentTile.x),
          static_cast<float>(nextTile.y) - static_cast<float>(currentTile.y)};

      if (direction.lengthSquared() != 0)
        direction = direction.normalized();

      e.update(dt, direction);
    }
  }
}

sf::Vector2u Level::findPathTo(sf::Vector2u startTile,
                               sf::Vector2u endTile) const {
  return {0, 0};
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
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
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

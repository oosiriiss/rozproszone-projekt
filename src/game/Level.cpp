#include <SFML/Graphics/RectangleShape.hpp>
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
    ASSERT(!"unreachable");
    return sf::Color(0, 0, 0, 0);
  }
}

Level::Level() : m_tiles{} {

  LOG_INFO("Loading default level data");
  loadLevel(this->Map1Data);
}
Level::Level(const MapData &tilemap) : m_tiles({}) { loadLevel(tilemap); }
Level::~Level() { LOG_INFO("Destroying level"); }

void Level::draw(sf::RenderWindow &window) const {
  for (const auto &tile : this->m_tiles) {
    window.draw(tile.rect);
  }
  for (const auto &e : m_enemies) {
    e.draw(window);
  }
}

void Level::loadLevel(const Level::MapData &data) {
  ASSERT(this->m_tiles.max_size() == data.tiles.max_size());

  this->m_currentMapID = data.id;

  LOG_DEBUG("Loading level: ", (int)data.id);

  for (int i = 0; i < m_tiles.max_size(); ++i) {

    const int col = i % MAP_WIDTH;
    const int row = i / MAP_WIDTH;

    const int x = col * TILE_SIZE;
    const int y = row * TILE_SIZE;

    TileType tile = data.tiles[i];

    this->m_tiles[i] = Tile(x, y, Level::TILE_SIZE, tile);

    if (tile == TileType::EnemySpawner) {
      m_spawners.push_back(EnemySpawner(2, 3.f, [this, x, y]() {
        //LOG_DEBUG("Spawning enemy");
        m_enemies.push_back(Enemy(x, y));
      }));
    }
  }

  LOG_DEBUG("Level loaded");
}

void Level::update(float dt) {

  for (auto &s : m_spawners) {
    s.update(dt);
  }

  for (auto &e : m_enemies) {
    e.update(dt);
  }
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

  for (const Tile &t : this->m_tiles) {

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

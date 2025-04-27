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
  case TileType::Count:
    ASSERT(!"unreachable");
    return sf::Color(0, 0, 0, 0);
  }
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
}

void Level::loadLevel(const Level::MapData &data) {
  ASSERT(this->tiles.max_size() == data.tiles.max_size());

  this->m_currentMapID = data.id;

  LOG_DEBUG("Loading level: ", (int)data.id);

  for (int i = 0; i < tiles.max_size(); ++i) {

    const int col = i % MAP_WIDTH;
    const int row = i / MAP_HEIGHT;

    const int x = col * TILE_SIZE;
    const int y = row * TILE_SIZE;

    this->tiles[i] = Tile(x, y, Level::TILE_SIZE, data.tiles[i]);
  }

  LOG_DEBUG("Level loaded");
}

void Level::update(GameWorld &world) {}

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
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
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
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   })
};
// clang-format on

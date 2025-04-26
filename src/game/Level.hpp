#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <initializer_list>

class GameWorld;

enum class TileType : int {
  //
  Ground,
  Wall,
  Count // Helper to easily find the number of elements in enum Should be always
        // last
};

struct Tile {

  Tile();
  Tile(float x, float y, float tileSize, TileType type);
  ~Tile() = default;

  // Sets the
  void update(float x, float y, TileType type);

  sf::RectangleShape rect;
  TileType type;

  static sf::Color getColor(TileType type);
};

struct Level {

  // Number of tiles in the map (horizontally)
  constexpr static int MAP_WIDTH = 32;

  // Number of tiles in the map (vertically)
  constexpr static int MAP_HEIGHT = 32;

  // Number of pixels taken by each tile (square tiles)
  constexpr static int TILE_SIZE = 32;

  typedef std::array<TileType, MAP_WIDTH * MAP_HEIGHT> MapData;

  Level(const MapData &initalMap);
  ~Level();

  void draw(sf::RenderWindow &window) const;
  void loadLevel(const MapData &data);
  void update(GameWorld &world);

  std::array<Tile, MAP_WIDTH * MAP_HEIGHT> tiles;

  const static MapData Map1Data;
};

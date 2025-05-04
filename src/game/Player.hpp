#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "../debug.hpp"

class GameWorld;

enum class Direction { Up, Down, Left, Right };

constexpr sf::Vector2f toVec(Direction dir) {
  switch (dir) {
  case Direction::Up:
    return {0, -1.f};
  case Direction::Down:
    return {0, 1.f};
  case Direction::Left:
    return {-1.f, 0};
  case Direction::Right:
    return {1.f, 0};
  }

  UNREACHABLE;
}

struct Player {

  Player();
  Player(int32_t id);

  void draw(sf::RenderWindow &window) const;
  void update();

  sf::RectangleShape rect;
  int32_t id;
};

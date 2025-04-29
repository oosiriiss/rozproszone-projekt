#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
class GameWorld;

enum class Direction { Up, Down, Left, Right };

struct Player {

  Player();
  Player(int32_t id);

  void draw(sf::RenderWindow &window) const;
  void update();
  void move(Direction dir);

  sf::RectangleShape rect;
  int32_t id;
};

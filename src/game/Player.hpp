#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
class GameWorld;

struct Player {

  Player();
  Player(uint8_t id);

  void draw(sf::RenderWindow &window) const;
  void update();

  sf::RectangleShape rect;
  uint8_t id;
};

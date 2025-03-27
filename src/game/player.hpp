#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
struct Player {

  Player();
  Player(uint8_t id);

  sf::RectangleShape rect;
  uint8_t id;
};

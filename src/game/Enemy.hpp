#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>

class GameWorld;

struct Enemy {

  Enemy(float x, float y);
  ~Enemy();

  void draw(sf::RenderWindow &window);
  void update(GameWorld &world);

  sf::CircleShape rect;
};

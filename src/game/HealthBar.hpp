#pragma once
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

struct HealthBar {

  HealthBar() = default;
  HealthBar(sf::FloatRect pos, int maxHealth);
  ~HealthBar() = default;

  void update(sf::FloatRect parentPos);
  void draw(sf::RenderWindow &window) const;

  int health;
  int maxHealth;

  sf::RectangleShape rect;
  sf::RectangleShape rectBG;

  static const float MAX_WIDTH;
  static const float HEIGHT;
};

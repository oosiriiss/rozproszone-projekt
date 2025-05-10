#pragma once
#include "HealthBar.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

struct Base {

  Base() = default;
  Base(sf::Vector2f pos);
  ~Base() = default;

  void update(float dt);
  void draw(sf::RenderWindow &window) const;
  void damage();

  sf::RectangleShape rect;
  HealthBar healthbar;

private:
  float m_dmgCooldown;
};

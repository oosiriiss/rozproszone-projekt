#include "Fireball.hpp"
#include "Level.hpp"

#include "../debug.hpp"

Fireball::Fireball() { direction = {0.f, 0.f}; }

Fireball::Fireball(sf::Vector2f startPos, sf::Vector2f direction)
    : rect(), direction(direction) {
  rect.setPosition(startPos);
  rect.setRadius(Level::TILE_SIZE / 2.f);
}

void Fireball::update(float dt) {
  sf::Vector2f vel = direction * dt * 10.f;
  this->rect.setPosition(this->rect.getPosition() + vel);
}

void Fireball::draw(sf::RenderWindow &window) const {
  window.draw(this->rect);
  DEBUG_OUTLINE(
      window, 
      this->rect.getPosition(),
      sf::Vector2f(this->rect.getRadius() * 2, this->rect.getRadius() * 2)
   );
}

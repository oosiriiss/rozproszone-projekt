#include "Base.hpp"
#include "Level.hpp"
#include <SFML/Window/Mouse.hpp>

Base::Base(sf::Vector2f pos) : healthbar(this->rect.getGlobalBounds(), 100) {
  this->rect.setPosition(pos);
  this->rect.setSize({Level::TILE_SIZE, Level::TILE_SIZE});
  this->rect.setFillColor(sf::Color::Blue);

  this->healthbar.update(this->rect.getGlobalBounds());

  m_dmgCooldown = 0.f;
}

void Base::update(float dt) {
  m_dmgCooldown -= dt;

  this->healthbar.update(this->rect.getGlobalBounds());
}

void Base::damage() {

  if (m_dmgCooldown <= 0) {
    this->healthbar.health -= 10;
    m_dmgCooldown = 2.f;
  }
}

void Base::draw(sf::RenderWindow &window) const {
  window.draw(this->rect);
  this->healthbar.draw(window);
}

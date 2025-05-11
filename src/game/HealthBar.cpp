#include "HealthBar.hpp"
#include "Level.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

const float HealthBar::MAX_WIDTH = Level::TILE_SIZE;
const float HealthBar::HEIGHT = 3.f;

HealthBar::HealthBar(sf::FloatRect parent, int maxHealth) {

  this->maxHealth = maxHealth;
  this->health = maxHealth;

  this->rect.setSize({MAX_WIDTH, HEIGHT});
  this->rect.setFillColor(sf::Color::Green);
  this->rectBG.setSize({MAX_WIDTH, HEIGHT});
  this->rectBG.setFillColor(sf::Color::Red);

  update(parent);
}

void HealthBar::update(sf::FloatRect parent) {

  sf::Vector2f pos = parent.position;
  pos.y -= HEIGHT;
  pos.x += parent.size.x / 2;
  pos.x -= MAX_WIDTH / 2;

  this->rect.setPosition(pos);
  this->rectBG.setPosition(pos);

  this->rect.setSize({(health * 1.f / maxHealth) * MAX_WIDTH, HEIGHT});
}

void HealthBar::draw(sf::RenderWindow &window) const {
  window.draw(this->rectBG);
  window.draw(this->rect);
}

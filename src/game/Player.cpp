#include "Player.hpp"
#include "Level.hpp"
#include <SFML/Window/Keyboard.hpp>

#include "../debug.hpp"

Player::Player() : rect(), id() {
  this->rect.setSize({Level::TILE_SIZE, Level::TILE_SIZE});
  this->rect.setFillColor(sf::Color::Red);
}
Player::Player(int32_t id) : id(id), rect() {
  this->rect.setSize({Level::TILE_SIZE, Level::TILE_SIZE});
  this->rect.setFillColor(sf::Color::Red);
}

void Player::draw(sf::RenderWindow &window) const {
  window.draw(this->rect);
  DEBUG_OUTLINE(window, rect.getPosition(), rect.getSize());
}

void Player::update() {}

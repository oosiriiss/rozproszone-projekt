#include "Player.hpp"
#include "../Application.hpp"
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

void Player::move(Direction dir) {
  if (dir == Direction::Up) {
    this->rect.move({0, -1.f});
  }
  if (dir == Direction::Down) {
    this->rect.move({0, 1.f});
  }
  if (dir == Direction::Left) {
    this->rect.move({-1.f, 0});
  }
  if (dir == Direction::Right) {
    this->rect.move({1.f, 0});
  }
}
void Player::update() {
}

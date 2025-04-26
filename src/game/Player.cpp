#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>

Player::Player() : rect(), id() {}
Player::Player(uint8_t id) : id(id), rect() {}

void Player::draw(sf::RenderWindow &window) const { window.draw(this->rect); }
void Player::update(GameWorld &world) {}
void Player::handleKeydown(sf::Keyboard::Key k) {

  if (k == sf::Keyboard::Key::W) {
    this->rect.move({0, -1.f});
  }
  if (k == sf::Keyboard::Key::S) {
    this->rect.move({0, 1.f});
  }
  if (k == sf::Keyboard::Key::A) {
    this->rect.move({-1.f, 0});
  }
  if (k == sf::Keyboard::Key::D) {
    this->rect.move({1.f, 0});
  }
}

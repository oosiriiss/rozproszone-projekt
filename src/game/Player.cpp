#include "Player.hpp"
#include "../Application.hpp"
#include "Level.hpp"
#include <SFML/Window/Keyboard.hpp>

#include "../debug.hpp"

Player::Player() : rect(), id() {
  this->rect.setSize({Level::TILE_SIZE, Level::TILE_SIZE});
  this->rect.setFillColor(sf::Color::Red);
}
Player::Player(uint8_t id) : id(id), rect() {
  this->rect.setSize({Level::TILE_SIZE, Level::TILE_SIZE});
  this->rect.setFillColor(sf::Color::Red);
}

void Player::draw(sf::RenderWindow &window) const {
  window.draw(this->rect);
  DEBUG_OUTLINE(window, rect.getPosition(), rect.getSize());
}
void Player::update() {

  if (Application::isKeyPressed(sf::Keyboard::Key::W)) {
    this->rect.move({0, -1.f});
  }
  if (Application::isKeyPressed(sf::Keyboard::Key::S)) {
    this->rect.move({0, 1.f});
  }
  if (Application::isKeyPressed(sf::Keyboard::Key::A)) {
    this->rect.move({-1.f, 0});
  }
  if (Application::isKeyPressed(sf::Keyboard::Key::D)) {
    this->rect.move({1.f, 0});
  }
}

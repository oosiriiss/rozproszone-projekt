#include "Enemy.hpp"

Enemy::Enemy(float x, float y) : rect() {

  this->rect.setPosition({x, y});
  this->rect.setRadius(13);
  this->rect.setFillColor(sf::Color::Red);
}
Enemy::~Enemy() {}

void Enemy::draw(sf::RenderWindow &window) { window.draw(this->rect); }
void Enemy::update(GameWorld &world) {
}

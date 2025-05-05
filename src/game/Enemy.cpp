#include "Enemy.hpp"
#include "Level.hpp"
#include <cstdlib>

Enemy::Enemy(float x, float y) : rect() {
  this->rect.setPosition({x, y});
  this->rect.setRadius(13);
  this->rect.setFillColor(sf::Color::Red);
}
Enemy::~Enemy() {}

void Enemy::draw(sf::RenderWindow &window) const { window.draw(this->rect); }
void Enemy::update(float dt, sf::Vector2f direction) {
  this->rect.move(direction * dt * 5.f);
}

EnemySpawner::EnemySpawner(uint32_t enemiesToSpawn, float spawnDelaySeconds,
                           std::function<void(void)> spawnCallback)
    : m_leftToSpawn(enemiesToSpawn), m_spawnDelaySeconds(spawnDelaySeconds),
      m_spawn(spawnCallback) {}
EnemySpawner::~EnemySpawner() {}

void EnemySpawner::update(float dt) {
  m_spawnTimer += dt;

  if (m_leftToSpawn > 0 && m_spawnTimer >= m_spawnDelaySeconds) {
    m_spawn();

    --m_leftToSpawn;
    m_spawnTimer = 0;
  }
}

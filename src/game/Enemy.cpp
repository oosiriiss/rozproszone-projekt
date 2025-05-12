#include "Enemy.hpp"
#include "HealthBar.hpp"
#include "Level.hpp"

Enemy::Enemy(sf::Vector2f startPos, sf::Vector2f destination)
    : rect(), healthBar({startPos, {Level::TILE_SIZE, Level::TILE_SIZE}}, 100) {
  this->rect.setPosition(startPos);
  this->rect.setRadius(Level::TILE_SIZE / 2.f);
  this->rect.setFillColor(sf::Color::Magenta);

  this->destination = destination;
}
Enemy::~Enemy() {}

void Enemy::draw(sf::RenderWindow &window) const {

  window.draw(this->rect);
  this->healthBar.draw(window);
}

void Enemy::update(float dt) {

  sf::Vector2f direction =
      (this->destination - this->rect.getPosition()).normalized();

  this->rect.move(direction * dt * 20.f);

  this->healthBar.update({this->rect.getPosition(),
                          {this->rect.getRadius(), this->rect.getRadius()}});
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

bool EnemySpawner::isDoneSpawning() const { return m_leftToSpawn == 0; }

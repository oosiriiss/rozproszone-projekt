#pragma once
#include "HealthBar.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <atomic>
#include <functional>

struct Level;

struct Enemy {

  Enemy(sf::Vector2f startPos, sf::Vector2f destination);
  ~Enemy();

  void draw(sf::RenderWindow &window) const;
  void update(float dt);

  sf::CircleShape rect;
  sf::Vector2f destination;
  HealthBar healthBar;

  struct DTO {
    sf::Vector2f pos;
    sf::Vector2f destination;
    int health;
  };
};

struct EnemySpawner {
  EnemySpawner(uint32_t enemiesToSpawn, float spawnDelaySeconds,
               std::function<void(void)> spawnCallback);
  ~EnemySpawner();

  void update(float dt);
  bool isDoneSpawning() const;

private:
  uint32_t m_leftToSpawn = 0;
  float m_spawnTimer = 0.f;
  float m_spawnDelaySeconds;

  std::function<void()> m_spawn;
};

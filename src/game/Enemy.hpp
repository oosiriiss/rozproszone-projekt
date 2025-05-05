#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <functional>

struct Enemy {

  Enemy(float x, float y);
  ~Enemy();

  void draw(sf::RenderWindow &window) const;
  void update(float dt, sf::Vector2f direction);

  sf::CircleShape rect;
};

struct EnemySpawner {
  EnemySpawner(uint32_t enemiesToSpawn, float spawnDelaySeconds,
               std::function<void(void)> spawnCallback);
  ~EnemySpawner();

  void update(float dt);

private:
  uint32_t m_leftToSpawn = 0;
  float m_spawnTimer = 0.f;

  float m_spawnDelaySeconds;

  std::function<void()> m_spawn;
};

#pragma once

#include <vector>

#include "Enemy.hpp"
#include "Level.hpp"

struct GameWorld {

  GameWorld();
  ~GameWorld();

  Level currentLevel;

  std::vector<Enemy> enemies;
};

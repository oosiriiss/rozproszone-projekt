#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
class Fireball {

  Fireball(sf::Vector2f startPos, sf::Vector2f direction);
  ~Fireball();

  void update(float dt);
  void draw(sf::RenderWindow &window) const;

  sf::CircleShape rect;
  sf::Vector2f direction;
};

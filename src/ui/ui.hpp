#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Main.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <functional>

#include "../AssetManager.hpp"

namespace ui {

#ifdef DEBUG_BUILD
#define DEBUG_OUTLINE(pos, size)                                               \
  sf::RectangleShape rect;                                                     \
  rect.setPosition(pos);                                                       \
  rect.setSize(size);                                                          \
  rect.setFillColor(sf::Color::Transparent);                                   \
  rect.setOutlineColor(sf::Color::Red);                                        \
  rect.setOutlineThickness(1.f);                                               \
  g_UIContext.window->draw(rect);
#else
#define DEBUG_OUTLINE(pos, size)
#endif
typedef int32_t ComponentID;

struct Context {
public:
  void init(sf::RenderWindow *window, const AssetManager *m);
  void endDraw();
  ComponentID getID();

  // returns text object with current text styles applied
  sf::Text getCurrentTextWithStyle(std::string_view text) const;

  // Returns position of where next comcponent should be placed
  sf::Vector2f nextPos(sf::Vector2f currentSize);

  void drawRect(sf::FloatRect rect, sf::Color color);
  static sf::Vector2f measureText(const sf::Text &text);

public:
  sf::Vector2i mousePos = {0, 0};
  bool didClickMouse = false;
  // user is about to interact with this component (eg. hover)
  ComponentID hot = -1;
  // user used this component (eg. clickd)
  ComponentID active = -1;

  sf::RenderWindow *window = nullptr;

private:
  sf::Vector2f m_currPos = {0.f, 0.f};
  ComponentID m_nextID = 0;

  const AssetManager *m_assetManager = nullptr;

  sf::RectangleShape m_helperRect;
};

inline Context g_UIContext;

// Returns true when clicked
bool Button(std::string_view text);
void Text(std::string_view text);

} // namespace ui

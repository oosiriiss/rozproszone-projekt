#include "ui.hpp"
#include "../logging.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <string_view>

#include "../debug.hpp"

namespace ui {

void Context::init(sf::RenderWindow *window, const AssetManager *m) {
  this->m_assetManager = m;
  this->window = window;
}

void Context::endDraw() {
  this->hot = -1;
  this->active = -1;
  this->mousePos = {0, 0};
  this->didClickMouse = false;
  this->m_currPos = {0.f, 0.f};
  this->m_nextID = 0;
}

ComponentID Context::getID() { return this->m_nextID++; }

sf::Text Context::getCurrentTextWithStyle(std::string_view value) const {
  sf::Text text(m_assetManager->getFont("font.ttf").font);

  const FontAsset &f = m_assetManager->getFont("font.ttf");
  text.setCharacterSize(20);
  text.setString(value.data());

  return text;
}

sf::Vector2f Context::nextPos(sf::Vector2f currentSize) {

  auto p = this->m_currPos;
  this->m_currPos.y += currentSize.y;

  return p;
}

void Context::drawRect(sf::FloatRect rect, sf::Color color) {
  m_helperRect.setPosition(rect.position);
  m_helperRect.setSize(rect.size);
  m_helperRect.setFillColor(color);
  this->window->draw(m_helperRect);
}

sf::Vector2f Context::measureText(const sf::Text &text) {
  sf::Vector2f textSize = text.getGlobalBounds().size;
  textSize.y += text.getLocalBounds().size.y;
  return textSize;
}

bool isClicked(ComponentID id, sf::FloatRect rect) {

  if (!g_UIContext.didClickMouse)
    return false;

  LOG_DEBUG("Component: ", id);

  const auto &s = rect.size;
  const auto &p = rect.position;

  const auto &clickPos = g_UIContext.mousePos;

  LOG_DEBUG("clickpos:", clickPos.x, " ", clickPos.y);
  LOG_DEBUG("rect:", p.x, " ", p.y, " rect size: ", s.x, " ", s.y);

  bool isClick = clickPos.y >= p.y && clickPos.y <= p.y + s.y &&
                 clickPos.x >= p.x && clickPos.x <= p.x + s.x;

  LOG_DEBUG("click ", clickPos.y >= p.y, " ", clickPos.y <= p.y + s.y, " ",
            clickPos.x >= p.x, " ", clickPos.x <= p.x + s.x);

  if (isClick && g_UIContext.active != id) {
    g_UIContext.active = id;

    return true;
  }
  return false;
}

bool Button(std::string_view text) {
  ComponentID thisID = g_UIContext.getID();

  sf::Text t = g_UIContext.getCurrentTextWithStyle(text);
  sf::Vector2f textSize = Context::measureText(t);
  sf::Vector2f pos = g_UIContext.nextPos(textSize);

  DEBUG_OUTLINE_PTR(g_UIContext.window, pos, textSize);

  t.setPosition(pos);

  // bg
  g_UIContext.drawRect({pos, textSize}, sf::Color(50, 50, 50));
  // text
  g_UIContext.window->draw(t);

  return isClicked(thisID, {pos, textSize});
}

void Text(std::string_view text) {
  ComponentID thisID = g_UIContext.getID();

  sf::Text t = g_UIContext.getCurrentTextWithStyle(text);
  sf::Vector2f textSize = Context::measureText(t);
  sf::Vector2f pos = g_UIContext.nextPos(textSize);

  DEBUG_OUTLINE_PTR(g_UIContext.window, pos, textSize);

  t.setPosition(pos);

  g_UIContext.window->draw(t);
}

} // namespace ui

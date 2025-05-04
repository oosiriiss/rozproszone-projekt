#pragma once

#ifdef DEBUG_BUILD
#include <iostream>

inline void __assertFail(const char *file, int line, const char *expression) {
  std::cout << "ASSERTION FAILED\n"
            << file << ":" << line << " Expression: " << expression << '\n';
  exit(-1);
}

#define ASSERT(expr)                                                           \
  (static_cast<bool>(expr) ? void(0) : __assertFail(__FILE__, __LINE__, #expr))
#define UNREACHABLE                                                            \
  ASSERT(false && "UNREACHABLE CODE REACHED");                                 \
  std::unreachable();

#define DEBUG_ONLY(x) x

#define DEBUG_OUTLINE(window, pos, size)                                       \
  sf::RectangleShape rect;                                                     \
  rect.setPosition(pos);                                                       \
  rect.setSize(size);                                                          \
  rect.setFillColor(sf::Color::Transparent);                                   \
  rect.setOutlineColor(sf::Color::Red);                                        \
  rect.setOutlineThickness(1.f);                                               \
  window.draw(rect);

#define DEBUG_OUTLINE_PTR(windowptr, pos, size)                                \
  DEBUG_OUTLINE((*windowptr), pos, size)

#define VECTOR_STR(v) "(", v.x, ", ", v.y, ")"

#else

#define ASSERT(expr)
#define UNREACHABLE std::unreachable();
#define DEBUG_ONLY(x)
#define DEBUG_OUTLINE(window, pos, size)
#define DEBUG_OUTLINE_PTR(windowptr, pos, size)
#define VECTOR_STR(v)

#endif

#pragma once

#ifdef DEBUG_BUILD
#include <iostream>
#include <utility>

constexpr void __assertFail(const char *file, int line,
                            const char *expression) {
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
#else
#define ASSERT(expr)
#define UNREACHABLE std::unreachable();
#define DEBUG_ONLY(x)
#endif

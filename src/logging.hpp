#pragma once
#include <cstdio>
#include <iostream>

// Debug logging available only in debug compile
#ifdef DEBUG_BUILD
#define LOG_DEBUG(...)                                                         \
  log("[", "\033[33m", "DEBUG", "\033[0m", "] ", __FILE_NAME__, ':', __LINE__, \
      " ", __VA_ARGS__)
#else
#define LOG_DEBUG(...)

#endif

// These represent log messages
#define LOG_ERROR(...)                                                         \
  log("[", "\033[31m", "ERROR", "\033[0m", "] ", __VA_ARGS__)

#define LOG_INFO(...) log("[", "\033[34m", "INFO", "\033[0m", "] ", __VA_ARGS__)

template <typename... ARGS> constexpr void log(ARGS... args) {
  (std::cout << ... << args) << std::endl;
}

#pragma once
#include <cstdint>
#include <cstdio>
#include <iostream>

#define LOG_ERROR(...)                                                           \
  logError(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...)                                                          \
  logInfo(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)

template <class... ARGS>
constexpr void logError(const char *file, int32_t line, const char *function,
                        ARGS... args) {

  std::cerr << file << ":" << line << " | " << function << " || ";
  (std::cerr << ... << args) << '\n';
}

template <class... ARGS>
constexpr void logInfo(const char *file, int32_t line, const char *function,
                       ARGS... args) {

  std::cout << file << ":" << line << " | " << function << " || ";
  (std::cout << ... << args) << '\n';
}

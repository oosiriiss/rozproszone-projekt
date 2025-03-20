#pragma once
#include <cstdint>
#include <cstdio>
#include <iostream>

#define LOG_ERROR(...) logError(__LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...) logInfo(__LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)

template <class... ARGS>
constexpr void logError(int32_t line, const char *function, ARGS... args) {

  std::cerr << ":" << line << " | " << function << " || ";
  (std::cerr << ... << args) << '\n';
}

template <class... ARGS>
constexpr void logInfo(int32_t line, const char *function, ARGS... args) {

  std::cout << ":" << line << " | " << function << " || ";
  (std::cout << ... << args) << '\n';
}

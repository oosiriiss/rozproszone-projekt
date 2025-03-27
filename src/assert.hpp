#include <iostream>

#define ASSERT_HELPER(s, xval, yval, msg, FUNC, ...) FUNC
#define assert2(xval, yval) assertEq(__FILE__, __LINE__, xval, yval, "")
#define assert3(xval, yval, msg) assertEq(__FILE__, __LINE__, xval, yval, msg)

#define assertEqual(...)                                                       \
  ASSERT_HELPER(, ##__VA_ARGS__, assert3(__VA_ARGS__), assert2(__VA_ARGS__))

// asserts x == y, crashes the app otherwise
void assertEq(const char *file, int line, auto expected, auto actual,
              std::string_view msg) {
  if (expected != actual) {

    std::cout << "ASSERTION FAILED\n"
              << file << ":" << line << " Actual: " << actual
              << " expected: " << expected;
    if (msg.size() > 0) {
      std::cout << " message: " << msg << '\n';
    }
    exit(-1);
  }
}

#pragma once
#include <string>

class Assert
{
public:
  static void ThrowIfTrue(bool condition, const std::string& errorMessage);
  static void ThrowIfFalse(bool condition, const std::string& errorMessage);
};

#if defined(NDEBUG) && defined(ASSERT_ON)
#define ASSERT_FALSE(x, msg) Assert::ThrowIfTrue(x, msg)
#define ASSERT_TRUE(x, msg) Assert::ThrowIfFalse(x, msg)
#else
#define ASSERT_TRUE(x, msg)
#define ASSERT_FALSE(x, msg)
#endif

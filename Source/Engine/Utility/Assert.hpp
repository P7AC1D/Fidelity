#pragma once
#include <string>

class Assert
{
public:
  static void ThrowIfTrue(bool condition, const std::string& errorMessage);
  static void ThrowIfFalse(bool condition, const std::string& errorMessage);
};

#ifdef _DEBUG 
#define NDEBUG
#endif

#if defined(NDEBUG) && defined(ASSERT_ENABLED)
#define ASSERT_FALSE(x, msg) Assert::ThrowIfTrue(x, msg)
#define ASSERT_TRUE(x, msg) Assert::ThrowIfFalse(x, msg)
#else
#define ASSERT_TRUE(x, msg)
#define ASSERT_FALSE(x, msg)
#endif

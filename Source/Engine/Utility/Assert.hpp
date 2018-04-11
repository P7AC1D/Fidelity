#pragma once
#include <string>

class Assert
{
public:
  static void ThrowIfTrue(bool condition, const std::string& errorMessage);
  static void ThrowIfFalse(bool condition, const std::string& errorMessage);
};

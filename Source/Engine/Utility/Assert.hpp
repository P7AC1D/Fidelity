#pragma once
#include <string>

class Assert
{
public:
  static void ThrowIfTrue(bool condition, const std::string& errorMessage);
};

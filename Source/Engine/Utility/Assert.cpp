#include "Assert.hpp"
#include <stdexcept>

void Assert::ThrowIfTrue(bool condition, const std::string& errorMessage)
{
  if (condition)
  {
    throw std::runtime_error(errorMessage);
  }
}

void Assert::ThrowIfFalse(bool condition, const std::string& errorMessage)
{
  if (!condition)
  {
    throw std::runtime_error(errorMessage);
  }
}

#include "Assert.hpp"
#include <stdexcept>

void Assert::throwIfTrue(bool condition, const std::string &errorMessage)
{
  if (condition)
  {
    throw std::runtime_error(errorMessage);
  }
}

void Assert::throwIfFalse(bool condition, const std::string &errorMessage)
{
  if (!condition)
  {
    throw std::runtime_error(errorMessage);
  }
}

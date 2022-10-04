#include "catch.hpp"

#include "../Engine/Maths/Math.hpp"

TEST_CASE("ROUND TO EVEN")
{
  REQUIRE(Math::RoundToEven(0.1f) == 0.0f);
  REQUIRE(Math::RoundToEven(0.5f) == 0.0f);
  REQUIRE(Math::RoundToEven(0.9f) == 1.0f);
  REQUIRE(Math::RoundToEven(1.1f) == 1.0f);
  REQUIRE(Math::RoundToEven(1.5f) == 2.0f);
  REQUIRE(Math::RoundToEven(1.9f) == 2.0f);
  REQUIRE(Math::RoundToEven(2.1f) == 2.0f);
  REQUIRE(Math::RoundToEven(2.5f) == 2.0f);
  REQUIRE(Math::RoundToEven(2.9f) == 3.0f);
}
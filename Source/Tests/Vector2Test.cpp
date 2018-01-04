#include "catch.hpp"

#include <cmath>

#include "../Engine/Maths/Vector2.hpp"

TEST_CASE("Vector2 Operations: ")
{
  float32 k = 3.0f;
  Vector2 vecA(4);
  Vector2 vecB(7, 3);
  
  SECTION("Constructor and Accessor")
  {
    Vector2 vecC;
    REQUIRE(vecA[0] == 4);
    REQUIRE(vecA[1] == 4);
    REQUIRE(vecB[0] == 7);
    REQUIRE(vecB[1] == 3);
    REQUIRE(vecC[0] == 0);
    REQUIRE(vecC[1] == 0);
  }
  
  SECTION("Asignment Constructor")
  {
    Vector2 vecC(vecA);
    REQUIRE(vecC[0] == vecA[0]);
    REQUIRE(vecC[1] == vecA[1]);
  }
  
  SECTION("Copy Assignment")
  {
    Vector2 vecC = vecA;
    REQUIRE(vecC[0] == vecA[0]);
    
    vecC = Vector2(4, 3);
    REQUIRE(vecC[0] == 4);
    REQUIRE(vecC[1] == 3);
  }
  
  SECTION("Dot Product")
  {
    auto dotProduct = Vector2::Dot(vecA, vecB);
    REQUIRE(dotProduct == 40);
  }
  
  SECTION("Addition Assignment")
  {
    Vector2 sum = vecA + vecB;
    REQUIRE(sum[0] == 11);
    REQUIRE(sum[1] == 7);
  }
  
  SECTION("Subtraction Assignment")
  {
    Vector2 diff = vecA - vecB;
    REQUIRE(diff[0] == -3);
    REQUIRE(diff[1] == 1);
  }
  
  SECTION("Scalar Addition-Assignment")
  {
    Vector2 sum = vecA + k;
    REQUIRE(sum[0] == 7);
    REQUIRE(sum[1] == 7);
    
    Vector2 sumB = k + vecB;
    REQUIRE(sumB[0] == 10);
    REQUIRE(sumB[1] == 6);
  }
  
  SECTION("Scalar Difference-Assignment")
  {
    Vector2 diff = vecA - k;
    REQUIRE(diff[0] == 1);
    REQUIRE(diff[1] == 1);
    
    Vector2 diffB = k - vecB;
    REQUIRE(diffB[0] == -4);
    REQUIRE(diffB[1] == 0);
  }
  
  SECTION("Scalar Multiplication-Assignment")
  {
    Vector2 product = vecA * k;
    REQUIRE(product[0] == 12);
    REQUIRE(product[1] == 12);
    
    Vector2 productScalar = k * vecB;
    REQUIRE(productScalar[0] == 21);
    REQUIRE(productScalar[1] == 9);
  }
  
  SECTION("Addition")
  {
    vecA+= vecB;
    REQUIRE(vecA[0] == 11);
    REQUIRE(vecA[1] == 7);
  }
  
  SECTION("Subtraction")
  {
    vecA-= vecB;
    REQUIRE(vecA[0] == -3);
    REQUIRE(vecA[1] == 1);
  }
  
  SECTION("Scalar Sum")
  {
    vecB+= k;
    REQUIRE(vecB[0] == 10);
    REQUIRE(vecB[1] == 6);
  }
  
  SECTION("Scalar Difference")
  {
    vecB-= k;
    REQUIRE(vecB[0] == 4);
    REQUIRE(vecB[1] == 0);
  }
  
  SECTION("Scalar Multiplication")
  {
    vecA *= k;
    REQUIRE(vecA[0] == 12);
    REQUIRE(vecA[1] == 12);
  }
  
  SECTION("Equality Comparisons")
  {
    Vector2 vecC(7, 3);
    REQUIRE(vecB == vecC);
    REQUIRE(vecA != vecC);
  }
  
  SECTION("Length and Normalization")
  {
    float32 length = vecB.Length();
    REQUIRE(length == sqrtf(49 + 9));
    
    vecB.Normalize();
    float32 lengthInv = 1.0f / length;
    REQUIRE(vecB[0] == 7 * lengthInv);
    REQUIRE(vecB[1] == 3 * lengthInv);
  }
}

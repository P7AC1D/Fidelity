#include "catch.hpp"

#include <cmath>

#include "../Engine/Maths/Vector2.hpp"
#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Maths/Vector4.hpp"

TEST_CASE("Constructor and Accessor", "[Vector4]")
{
  Vector4 vecA;
  Vector4 vecB(1);
  Vector4 vecC(Vector2(3, 4), 5, 6);
  Vector4 vecD(Vector3(5, 6, 7), 8);
  Vector4 vecE(vecD);
  
  REQUIRE(vecA[0] == 0);
  REQUIRE(vecA[1] == 0);
  REQUIRE(vecA[2] == 0);
  REQUIRE(vecA[3] == 0);
  
  REQUIRE(vecB[0] == 1);
  REQUIRE(vecB[1] == 1);
  REQUIRE(vecB[2] == 1);
  REQUIRE(vecB[3] == 1);
  
  REQUIRE(vecC[0] == 3);
  REQUIRE(vecC[1] == 4);
  REQUIRE(vecC[2] == 5);
  REQUIRE(vecC[3] == 6);
  
  REQUIRE(vecD[0] == 5);
  REQUIRE(vecD[1] == 6);
  REQUIRE(vecD[2] == 7);
  REQUIRE(vecD[3] == 8);
  
  REQUIRE(vecE[0] == 5);
  REQUIRE(vecE[1] == 6);
  REQUIRE(vecE[2] == 7);
  REQUIRE(vecE[3] == 8);
}

TEST_CASE("Assignment Operator", "[Vector4]")
{
  Vector4 vecA(1,2,3,4);
  Vector4 vecB = vecA;
  REQUIRE(vecB[0] == vecA[0]);
  REQUIRE(vecB[1] == vecA[1]);
  REQUIRE(vecB[2] == vecA[2]);
  REQUIRE(vecB[3] == vecA[3]);
}

TEST_CASE("Binary Operators", "[Vector4]")
{
  Vector4 vecA(1, 2, 3, 4);
  Vector4 vecB(5, 6, 7, 8);
  
  SECTION("Addition")
  {
    Vector4 vecC = vecA + vecB;
    REQUIRE(vecC[0] == 6);
    REQUIRE(vecC[1] == 8);
    REQUIRE(vecC[2] == 10);
    REQUIRE(vecC[3] == 12);
  }
  
  SECTION("Addition-Assignment")
  {
    vecB += vecA;
    REQUIRE(vecB[0] == 6);
    REQUIRE(vecB[1] == 8);
    REQUIRE(vecB[2] == 10);
    REQUIRE(vecB[3] == 12);
  }

  SECTION("Subtraction")
  {
    Vector4 vecC = vecA - vecB;
    REQUIRE(vecC[0] == -4);
    REQUIRE(vecC[1] == -4);
    REQUIRE(vecC[2] == -4);
    REQUIRE(vecC[3] == -4);
  }
  
  SECTION("Multiplication")
  {
    Vector4 vecC = vecA * vecB;
    REQUIRE(vecC[0] == 5);  // 1 * 5
    REQUIRE(vecC[1] == 12); // 2 * 6
    REQUIRE(vecC[2] == 21); // 3 * 7
    REQUIRE(vecC[3] == 32); // 4 * 8
  }
  
  SECTION("Division")
  {
    Vector4 vecC(12.0f, 15.0f, 18.0f, 20.0f);
    
    // Test scalar division
    Vector4 result = vecC / 3.0f;
    REQUIRE(result[0] == 4.0f);  // 12 / 3
    REQUIRE(result[1] == 5.0f);  // 15 / 3
    REQUIRE(result[2] == 6.0f);  // 18 / 3
    REQUIRE(result[3] == Approx(6.666667f).epsilon(0.001f));  // 20 / 3
  }
}

TEST_CASE("Scalar Binary Operators", "[Vector4]")
{
  Vector4 vecA(4, 5, 6, 7);
  float32 k = 10.0f;
  
  SECTION("Addition")
  {
    Vector4 vecB = vecA + k;
    REQUIRE(vecB[0] == 14);
    REQUIRE(vecB[1] == 15);
    REQUIRE(vecB[2] == 16);
    REQUIRE(vecB[3] == 17);
    
    Vector4 vecC = k + vecA;
    REQUIRE(vecC[0] == 14);
    REQUIRE(vecC[1] == 15);
    REQUIRE(vecC[2] == 16);
    REQUIRE(vecC[3] == 17);
  }
  
  SECTION("Addition-Assignment")
  {
    vecA += k;
    REQUIRE(vecA[0] == 14);
    REQUIRE(vecA[1] == 15);
    REQUIRE(vecA[2] == 16);
    REQUIRE(vecA[3] == 17);
  }
  
  SECTION("Subtraction")
  {
    Vector4 vecB = vecA - k;
    REQUIRE(vecB[0] == -6);
    REQUIRE(vecB[1] == -5);
    REQUIRE(vecB[2] == -4);
    REQUIRE(vecB[3] == -3);
    
    Vector4 vecC = k - vecA;
    REQUIRE(vecC[0] == 6);
    REQUIRE(vecC[1] == 5);
    REQUIRE(vecC[2] == 4);
    REQUIRE(vecC[3] == 3);
  }
  
  SECTION("Subtraction-Assignment")
  {
    vecA -= k;
    REQUIRE(vecA[0] == -6);
    REQUIRE(vecA[1] == -5);
    REQUIRE(vecA[2] == -4);
    REQUIRE(vecA[3] == -3);
  }
  
  SECTION("Multiplication")
  {
    Vector4 vecB = vecA * k;
    REQUIRE(vecB[0] == 40);
    REQUIRE(vecB[1] == 50);
    REQUIRE(vecB[2] == 60);
    REQUIRE(vecB[3] == 70);
    
    Vector4 vecC = k * vecA;
    REQUIRE(vecC[0] == 40);
    REQUIRE(vecC[1] == 50);
    REQUIRE(vecC[2] == 60);
    REQUIRE(vecC[3] == 70);
  }
  
  SECTION("Multiplication-Assignment")
  {
    vecA *= k;
    REQUIRE(vecA[0] == 40);
    REQUIRE(vecA[1] == 50);
    REQUIRE(vecA[2] == 60);
    REQUIRE(vecA[3] == 70);
  }
}

TEST_CASE("Relational Operators", "[Vector4]")
{
  Vector4 vecA(4, 2, 6, 7);
  Vector4 vecB(2, 6, 5, 5);
  REQUIRE(vecA == vecA);
  REQUIRE(vecB == vecB);
  REQUIRE(vecA != vecB);
  REQUIRE(vecB != vecA);
  
  // Test edge cases for equality
  Vector4 vecC(4, 2, 6, 7);
  REQUIRE(vecA == vecC);
  REQUIRE(!(vecA != vecC));
  
  // Test inequality thoroughly
  Vector4 vecD(4, 2, 6, 8); // Only W differs
  REQUIRE(vecA != vecD);
  REQUIRE(!(vecA == vecD));
}

TEST_CASE("Length and Normalization", "[Vector4]")
{
  Vector4 vecA(2, 2, 2, 2);
  float32 length = 4;
  REQUIRE(vecA.Length() == 4);
  
  vecA.Normalize();
  float32 lengthInv = 1.0f / length;
  REQUIRE(vecA[0] == 0.5f);
  REQUIRE(vecA[1] == 0.5f);
  REQUIRE(vecA[2] == 0.5f);
  REQUIRE(vecA[3] == 0.5f);
  
  // Test zero vector normalization safety
  Vector4 zeroVec(0.0f, 0.0f, 0.0f, 0.0f);
  Vector4 originalZero = zeroVec;
  zeroVec.Normalize();
  REQUIRE(zeroVec == originalZero); // Should remain unchanged
  
  Vector4 staticNormalized = Vector4::Normalize(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
  REQUIRE(staticNormalized == Vector4(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST_CASE("Vector4 Mathematical Properties", "[Vector4]")
{
  SECTION("Dot Product")
  {
    Vector4 vecA(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 vecB(5.0f, 6.0f, 7.0f, 8.0f);
    
    float32 dotProduct = Vector4::Dot(vecA, vecB);
    // 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
    REQUIRE(dotProduct == 70.0f);
    
    // Test commutativity
    float32 dotProductReverse = Vector4::Dot(vecB, vecA);
    REQUIRE(dotProduct == dotProductReverse);
    
    // Test orthogonal vectors
    Vector4 vecC(1.0f, 0.0f, 0.0f, 0.0f);
    Vector4 vecD(0.0f, 1.0f, 0.0f, 0.0f);
    REQUIRE(Vector4::Dot(vecC, vecD) == 0.0f);
  }
  
  SECTION("Static Methods")
  {
    Vector4 vec(3.0f, 4.0f, 0.0f, 0.0f);
    float32 length = Vector4::Length(vec);
    REQUIRE(length == 5.0f);  // sqrt(3*3 + 4*4) = 5
    
    Vector4 normalized = Vector4::Normalize(vec);
    REQUIRE(normalized[0] == Approx(0.6f).epsilon(0.0001f));   // 3/5
    REQUIRE(normalized[1] == Approx(0.8f).epsilon(0.0001f));   // 4/5
    REQUIRE(normalized[2] == 0.0f);
    REQUIRE(normalized[3] == 0.0f);
  }
}

TEST_CASE("Vector4 Division Operations", "[Vector4]")
{
  Vector4 vecA(8, 12, 16, 20);
  
  SECTION("Scalar Division")
  {
    Vector4 result = vecA / 4.0f;
    REQUIRE(result[0] == 2.0f);
    REQUIRE(result[1] == 3.0f);
    REQUIRE(result[2] == 4.0f);
    REQUIRE(result[3] == 5.0f);
    
    // Test division by zero safety
    Vector4 zeroDiv = vecA / 0.0f;
    REQUIRE(zeroDiv[0] == 0.0f);
    REQUIRE(zeroDiv[1] == 0.0f);
    REQUIRE(zeroDiv[2] == 0.0f);
    REQUIRE(zeroDiv[3] == 0.0f);
  }
}

TEST_CASE("Vector4 Inequality Operator Fix", "[Vector4]")
{
  Vector4 vecA(1, 2, 3, 4);
  Vector4 vecB(1, 2, 3, 4);
  Vector4 vecC(1, 2, 3, 5);
  
  // Test equality - should be true
  REQUIRE(vecA == vecB);
  
  // Test inequality - should be true
  REQUIRE(vecA != vecC);
  
  // Test inequality - should be false
  REQUIRE(!(vecA != vecB));
}

TEST_CASE("Vector4 Zero Vector Normalization", "[Vector4]")
{
  Vector4 zeroVec(0.0f, 0.0f, 0.0f, 0.0f);
  Vector4 normalizedZero = Vector4::Normalize(zeroVec);
  
  // Should remain zero after normalization attempt
  REQUIRE(normalizedZero[0] == 0.0f);
  REQUIRE(normalizedZero[1] == 0.0f);
  REQUIRE(normalizedZero[2] == 0.0f);
  REQUIRE(normalizedZero[3] == 0.0f);
  
  // Test instance normalize on zero vector
  Vector4 zeroVec2(0.0f, 0.0f, 0.0f, 0.0f);
  zeroVec2.Normalize();
  REQUIRE(zeroVec2[0] == 0.0f);
  REQUIRE(zeroVec2[1] == 0.0f);
  REQUIRE(zeroVec2[2] == 0.0f);
  REQUIRE(zeroVec2[3] == 0.0f);
}

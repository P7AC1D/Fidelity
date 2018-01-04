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
  
  SECTION("Subtraction-Assignment")
  {
    vecB -= vecA;
    REQUIRE(vecB[0] == 4);
    REQUIRE(vecB[1] == 4);
    REQUIRE(vecB[2] == 4);
    REQUIRE(vecB[3] == 4);
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
}

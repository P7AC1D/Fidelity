#include "catch.hpp"

#include "../Engine/Maths/Degree.hpp"
#include "../Engine/Maths/Math.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Radian.hpp"
#include "../Engine/Maths/Vector3.hpp"

TEST_CASE("Quaternion Constructor and Accessor")
{
  Quaternion qatA;
  REQUIRE(qatA[0] == 0);
  REQUIRE(qatA[1] == 0);
  REQUIRE(qatA[2] == 0);
  REQUIRE(qatA[3] == 0);
  
  Quaternion qatB(5, 2, 3, 4);
  REQUIRE(qatB[0] == 2);
  REQUIRE(qatB[1] == 3);
  REQUIRE(qatB[2] == 4);
  REQUIRE(qatB[3] == 5);
  
  Quaternion qatC(qatB);
  REQUIRE(qatC[0] == 2);
  REQUIRE(qatC[1] == 3);
  REQUIRE(qatC[2] == 4);
  REQUIRE(qatC[3] == 5);
  
  Degree angle(30);
  Vector3 axis(1, 1, 1);
  axis.Normalize();
  Quaternion qatD(axis, angle);
  REQUIRE(qatD[0] == Approx(0.149429f));
  REQUIRE(qatD[1] == Approx(0.149429f));
  REQUIRE(qatD[2] == Approx(0.149429f));
  REQUIRE(qatD[3] == Approx(0.965926f));
  
  Quaternion qatE(angle, angle, angle);
  REQUIRE(qatE[0] == Approx(0.1767767f));
  REQUIRE(qatE[1] == Approx(0.3061862f));
  REQUIRE(qatE[2] == Approx(0.3061862f));
  REQUIRE(qatE[3] == Approx(0.8838835f));
  
  Quaternion qatF = Quaternion::Identity;
  REQUIRE(qatF[0] == 0);
  REQUIRE(qatF[1] == 0);
  REQUIRE(qatF[2] == 0);
  REQUIRE(qatF[3] == 1);
  
  Quaternion qatG = Quaternion::Zero;
  REQUIRE(qatG[0] == 0);
  REQUIRE(qatG[1] == 0);
  REQUIRE(qatG[2] == 0);
  REQUIRE(qatG[3] == 0);
}

TEST_CASE("Quaterion Assignment Operators")
{
  Quaternion qatA = Quaternion(4, 3, 2, 5);
  REQUIRE(qatA[0] == 3);
  REQUIRE(qatA[1] == 2);
  REQUIRE(qatA[2] == 5);
  REQUIRE(qatA[3] == 4);
}

TEST_CASE("Quaternion Binary Operators")
{
  Quaternion qatA(4, 3, 2, 5);
  Quaternion qatB(5, 4, 4, 8);
  
  SECTION("Addition")
  {
    Quaternion qatC = qatA + qatB;
    REQUIRE(qatC[0] == 7);
    REQUIRE(qatC[1] == 6);
    REQUIRE(qatC[2] == 13);
    REQUIRE(qatC[3] == 9);
    
    Quaternion qatD = qatA + 0.5f;
    REQUIRE(qatD[0] == 3.5);
    REQUIRE(qatD[1] == 2.5);
    REQUIRE(qatD[2] == 5.5);
    REQUIRE(qatD[3] == 4.5);
  }
  
  SECTION("Subtraction")
  {
    Quaternion qatC = qatA - qatB;
    REQUIRE(qatC[0] == -1);
    REQUIRE(qatC[1] == -2);
    REQUIRE(qatC[2] == -3);
    REQUIRE(qatC[3] == -1);
    
    Quaternion qatD = qatA - 0.5;
    REQUIRE(qatD[0] == 2.5);
    REQUIRE(qatD[1] == 1.5);
    REQUIRE(qatD[2] == 4.5);
    REQUIRE(qatD[3] == 3.5);
  }
  
  SECTION("Multiplication")
  {
    Quaternion qatC = Quaternion(1, 0, 1, 0) * Quaternion(1, 0.5f, 0.5f, 0.75f);
    REQUIRE(qatC[0] == 1.25f);
    REQUIRE(qatC[1] == 1.5f);
    REQUIRE(qatC[2] == 0.25f);
    REQUIRE(qatC[3] == 0.5f);
    
    Quaternion qatD = qatA * 0.5;
    REQUIRE(qatD[0] == 1.5);
    REQUIRE(qatD[1] == 1);
    REQUIRE(qatD[2] == 2.5);
    REQUIRE(qatD[3] == 2);
  }
}

TEST_CASE("Quaternion Binary Assignment Operators")
{
  Quaternion qatA(4, 3, 2, 3);
  Quaternion qatB(6, 4, 3, 3);
  
  SECTION("Addition")
  {
    qatA += qatB;
    REQUIRE(qatA[0] == 7);
    REQUIRE(qatA[1] == 5);
    REQUIRE(qatA[2] == 6);
    REQUIRE(qatA[3] == 10);
    
    qatB += 2.0f;
    REQUIRE(qatB[0] == 6.0f);
    REQUIRE(qatB[1] == 5.0f);
    REQUIRE(qatB[2] == 5.0f);
    REQUIRE(qatB[3] == 8.0f);
  }
  
  SECTION("Subtraction")
  {
    qatA -= qatB;
    REQUIRE(qatA[0] == -1);
    REQUIRE(qatA[1] == -1);
    REQUIRE(qatA[2] == 0);
    REQUIRE(qatA[3] == -2);
    
    qatB -= 0.5f;
    REQUIRE(qatB[0] == 3.5);
    REQUIRE(qatB[1] == 2.5);
    REQUIRE(qatB[2] == 2.5);
    REQUIRE(qatB[3] == 5.5);
  }
}

TEST_CASE("Quaternion Comparison Operators")
{
  Quaternion qatA(4, 3, 6, 7);
  Quaternion qatB(8, 2, 2, 6);
  
  REQUIRE(qatA == qatA);
  REQUIRE(qatB == qatB);
  
  REQUIRE(qatA != qatB);
  REQUIRE(qatB != qatA);
}

TEST_CASE("Quaternion Normalize and Norm")
{
  Quaternion qat(4, 3, 6, 7);
  
  float32 norm = sqrtf(110);
  float32 normInv = 1.0f / norm;
  REQUIRE(qat.Norm() == norm);
  
  qat.Normalize();
  REQUIRE(qat[0] == 3.0f * normInv);
  REQUIRE(qat[1] == 6.0f * normInv);
  REQUIRE(qat[2] == 7.0f * normInv);
  REQUIRE(qat[3] == 4.0f * normInv);
  REQUIRE(qat.Norm() == Approx(1.0f));
}

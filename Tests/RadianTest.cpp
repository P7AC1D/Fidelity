#include "catch.hpp"

#include "Degree.hpp"
#include "Math.hpp"
#include "Radian.hpp"

TEST_CASE("Radian Constructors")
{
  float32 pi = (float32)Math::Pi;
  float32 halfPi = (float32)Math::HalfPi;
  
  Radian angleA;
  Radian angleB(Math::Pi);
  Radian angleC(Degree(90.0f));
  Radian angleD(angleB);
  
  REQUIRE(angleA.InRadians() == 0.0);
  REQUIRE(angleB.InRadians() == pi);
  REQUIRE(angleC.InRadians() == halfPi);
  REQUIRE(angleD.InRadians() == pi);
  
  REQUIRE(angleA.InDegrees() == 0.0f);
  REQUIRE(angleB.InDegrees() == 180.0f);
  REQUIRE(angleC.InDegrees() == 90.0f);
  REQUIRE(angleD.InDegrees() == 180.0f);
}

TEST_CASE("Radian Assignment operators")
{
  Radian angleA(1.0f);
  Degree angleB(45.0f);
  
  Radian angleC;
  angleC = angleA;
  REQUIRE(angleC.InRadians() == 1.0f);
  
  Radian angleD;
  angleD = angleB;
  REQUIRE(angleD.InDegrees() == 45.0f);
  
}

TEST_CASE("Radian Binary Operators")
{
  Radian angleA(3.0f);
  Radian angleB(1.5f);
  Degree angleC(30.0f);
  
  SECTION("Addition")
  {
    Radian angleE;
    angleE = angleA + 3.0f;
    REQUIRE(angleE.InRadians() == 6.0f);
    
    Radian angleF;
    angleF = angleA + angleB;
    REQUIRE(angleF.InRadians() == 4.5f);
    
    Radian angleG;
    angleG = angleA + angleC;
    REQUIRE(angleG.InRadians() == 3.0f + angleC.InRadians());
  }
  
  SECTION("Subtraction")
  {
    Radian angleD;
    angleD = angleA - angleB;
    REQUIRE(angleD.InRadians() == 1.5f);
    
    Radian angleE;
    angleE = angleA - angleC;
    REQUIRE(angleE.InRadians() == angleA.InRadians() - angleC.InRadians());
    
    Radian angleF;
    angleF = angleA - 2.0f;
    REQUIRE(angleF.InRadians() == 1.0f);
  }
  
  SECTION("Multiplication")
  {
    Radian angleD;
    angleD = angleA * 2.5f;
    REQUIRE(angleD.InRadians() == 7.5);
    
    Radian angleE;
    angleE = angleA * angleB;
    REQUIRE(angleE.InRadians() == 3.0f * 1.5f);
    
    Radian angleF;
    angleF = angleA * angleC;
    REQUIRE(angleF.InRadians() == 3.0f * angleC.InRadians());
  }
  
}
  TEST_CASE("Radian Binary Assignment Operators")
  {
    Radian angleA(3.0f);
    Radian angleB(1.5f);
    Degree angleC(30.0f);
    
    SECTION("Addition")
    {
      Radian angleD;
      angleD += angleA;
      REQUIRE(angleD.InRadians() == angleA.InRadians());
      
      Radian angleE;
      angleE += angleC;
      REQUIRE(angleE.InRadians() == angleC.InRadians());
      
      Radian angleF;
      angleF += angleA;
      REQUIRE(angleF.InRadians() == angleA.InRadians());
    }
    
    SECTION("Subtraction")
    {
      Radian angleD;
      angleD -= angleA;
      REQUIRE(angleD.InRadians() == - angleA.InRadians());
      
      Radian angleE;
      angleE -= angleC;
      REQUIRE(angleE.InRadians() == - angleC.InRadians());
      
      Radian angleF;
      angleF -= angleA;
      REQUIRE(angleF.InRadians() == - angleA.InRadians());
    }
    
    SECTION("Multiplication")
    {
      Radian angleD(3.0);
      angleD *= angleA;
      REQUIRE(angleD.InRadians() == 9.0f);
      
      Radian angleE(2.5);
      angleE *= angleC;
      REQUIRE(angleE.InRadians() == 2.5f * angleC.InRadians());
      
      Radian angleF(2.0f);
      angleF *= angleA;
      REQUIRE(angleF.InRadians() == 6.0f);
    }
  }

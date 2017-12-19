#include "catch.hpp"

#include "Degree.hpp"
#include "Math.hpp"
#include "Radian.hpp"

TEST_CASE("Degree Constructors")
{
  float32 pi = (float32)Math::Pi;
  float32 halfPi = (float32)Math::HalfPi;
  
  Degree angleA;
  Degree angleB(180.0f);
  Degree angleC((Radian(halfPi)));
  Degree angleD(angleB);
  
  REQUIRE(angleA.InRadians() == 0.0);
  REQUIRE(angleB.InRadians() == pi);
  REQUIRE(angleC.InRadians() == halfPi);
  REQUIRE(angleD.InRadians() == pi);
  
  REQUIRE(angleA.InDegrees() == 0.0f);
  REQUIRE(angleB.InDegrees() == 180.0f);
  REQUIRE(angleC.InDegrees() == 90.0f);
  REQUIRE(angleD.InDegrees() == 180.0f);
}

TEST_CASE("Degree Assignment operators")
{
  float32 angle = 1.5f;
  Radian angleA(1.0f);
  Degree angleB(45.0f);
  
  Radian angleC = angleA;
  REQUIRE(angleC.InDegrees() == angleA.InDegrees());
  
  Radian angleD = angleB;
  REQUIRE(angleD.InDegrees() == 45.0f);
  
  Radian angleE = angle;
  REQUIRE(angleE.InDegrees() == 1.5f * Math::Rad2Deg);
}

TEST_CASE("Degree Binary Operators")
{
  Radian angleA(3.0f);
  Degree angleB(45.0f);
  Degree angleC(30.0f);
  
  SECTION("Addition")
  {
    Degree angleF = angleC + angleB;
    REQUIRE(angleF.InDegrees() == 75.0f);
    
    Degree angleG = angleB + angleC;
    REQUIRE(angleG.InDegrees() == angleB.InDegrees() + angleC.InDegrees());
  }
  
  SECTION("Subtraction")
  {
    Degree angleD = angleA - angleB;
    REQUIRE(angleD.InDegrees() == angleA.InDegrees() - 45.0f);
    
    Degree angleE = angleB - angleC;
    REQUIRE(angleE.InDegrees() == 15.0f);
  }
  
  SECTION("Multiplication")
  {
    Degree angleE = angleB * angleA;
    REQUIRE(angleE.InDegrees() == 45.0f * Degree(angleA).InDegrees());
    
    Degree angleF = angleB * angleC;
    REQUIRE(angleF.InDegrees() == 30.0f * 45.0f);
  }  
}

TEST_CASE("Degree Binary Assignment Operators")
{
  Radian angleA(3.0f);
  Degree angleB(1.5f);
  Degree angleC(30.0f);
  
  SECTION("Addition")
  {
    Degree angleD;
    angleD += angleA;
    REQUIRE(angleD.InDegrees() == angleA.InDegrees());
    
    Degree angleE;
    angleE += angleC;
    REQUIRE(angleE.InDegrees() == angleC.InDegrees());
  }
  
  SECTION("Subtraction")
  {
    Degree angleD;
    angleD -= angleA;
    REQUIRE(angleD.InDegrees() == - angleA.InDegrees());
    
    Degree angleE;
    angleE -= angleC;
    REQUIRE(angleE.InDegrees() == - angleC.InDegrees());
    
    Degree angleG;
    angleG -= angleC;
    REQUIRE(angleG.InDegrees() == - angleC.InDegrees());
  }
  
  SECTION("Multiplication")
  {
    Degree angleD(3.0f);
    angleD *= angleC;
    REQUIRE(angleD.InDegrees() == 90.0f);
    
    Degree angleE(10.0f);
    angleE *= angleC;
    REQUIRE(angleE.InDegrees() == 300.0f);
  }
}


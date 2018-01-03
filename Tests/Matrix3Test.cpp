#include "catch.hpp"

#include "Matrix3.hpp"
#include "Quaternion.hpp"
#include "Vector3.hpp"

TEST_CASE("Matrix3 Constructors and Accessors")
{
  SECTION("Zero")
  {
    Matrix3 matA(Matrix3::Zero);
    REQUIRE(matA[0][0] == 0.0f);
    REQUIRE(matA[0][1] == 0.0f);
    REQUIRE(matA[0][2] == 0.0f);
    REQUIRE(matA[1][0] == 0.0f);
    REQUIRE(matA[1][1] == 0.0f);
    REQUIRE(matA[1][2] == 0.0f);
    REQUIRE(matA[2][0] == 0.0f);
    REQUIRE(matA[2][1] == 0.0f);
    REQUIRE(matA[2][2] == 0.0f);
  }
  
  SECTION("Identity")
  {
    Matrix3 matA(Matrix3::Identity);
    REQUIRE(matA[0][0] == 1.0f);
    REQUIRE(matA[0][1] == 0.0f);
    REQUIRE(matA[0][2] == 0.0f);
    REQUIRE(matA[1][0] == 0.0f);
    REQUIRE(matA[1][1] == 1.0f);
    REQUIRE(matA[1][2] == 0.0f);
    REQUIRE(matA[2][0] == 0.0f);
    REQUIRE(matA[2][1] == 0.0f);
    REQUIRE(matA[2][2] == 1.0f);
  }
  
  SECTION("Full constructor")
  {
    float32 m00 = 3.0f;
    float32 m01 = 8.0f;
    float32 m02 = 2.2f;
    float32 m10 = 6.3f;
    float32 m11 = 2.1f;
    float32 m12 = 5.5f;
    float32 m20 = 1.0f;
    float32 m21 = 3.1f;
    float32 m22 = 6.9f;
    
    Matrix3 matA(m00, m01, m02,
                 m10, m11, m12,
                 m20, m21, m22);
    
    REQUIRE(matA[0][0] == m00);
    REQUIRE(matA[0][1] == m01);
    REQUIRE(matA[0][2] == m02);
    REQUIRE(matA[1][0] == m10);
    REQUIRE(matA[1][1] == m11);
    REQUIRE(matA[1][2] == m12);
    REQUIRE(matA[2][0] == m20);
    REQUIRE(matA[2][1] == m21);
    REQUIRE(matA[2][2] == m22);
  }
  
  SECTION("Quaternion Constructor")
  {
    Quaternion qat(2.2f, 4.4f, 2.0f, 6.2f);
    qat.Normalize();
    Matrix3 matA(qat);
    
    REQUIRE(matA[0][0] == Approx(-0.2737095f));
    REQUIRE(matA[0][1] == Approx(-0.1452581f));
    REQUIRE(matA[0][2] == Approx(0.9507803f));
    REQUIRE(matA[1][0] == Approx(0.6734694f));
    REQUIRE(matA[1][1] == Approx(-0.7346939f));
    REQUIRE(matA[1][2] == Approx(0.0816327f));
    REQUIRE(matA[2][0] == Approx(0.6866747f));
    REQUIRE(matA[2][1] == Approx(0.6626651f));
    REQUIRE(matA[2][2] == Approx(0.2989196f));
  }
}

TEST_CASE("Quaternion Equality Operators")
{
  Matrix3 matA(3.9f, 2.1f, 9.0f,
               7.7f, 8.3f, 5.7f,
               8.2f, 4.4f, 1.3f);
  REQUIRE(matA == matA);
  
  Matrix3 matB(2.2f, 9.4f, 3.0f,
               5.4, 5.2f, 2.78,
               6.4, 6.8, 2.5);
  REQUIRE(matA != matB);
  REQUIRE(matB != matA);
}

TEST_CASE("Quaternion Mathematical Operators")
{
  Matrix3 matA(3.9f, 2.1f, 9.0f,
               7.7f, 8.3f, 5.7f,
               8.2f, 4.4f, 1.3f);
  Matrix3 matB(2.1f, 5.5f, 9.2f,
               7.0f, 2.2f, 1.5f,
               4.7f, 3.0f, 2.1);
  
  SECTION("Addition")
  {
    Matrix3 matC = matA + matB;
    REQUIRE(matC[0][0] == Approx(6.0f));
    REQUIRE(matC[0][1] == Approx(7.6f));
    REQUIRE(matC[0][2] == Approx(18.2f));
    REQUIRE(matC[1][0] == Approx(14.7f));
    REQUIRE(matC[1][1] == Approx(10.5f));
    REQUIRE(matC[1][2] == Approx(7.2f));
    REQUIRE(matC[2][0] == Approx(12.9f));
    REQUIRE(matC[2][1] == Approx(7.4f));
    REQUIRE(matC[2][2] == Approx(3.4f));
    
    Matrix3 matD = matA + 2.4f;
    REQUIRE(matD[0][0] == Approx(6.3f));
    REQUIRE(matD[0][1] == Approx(4.5f));
    REQUIRE(matD[0][2] == Approx(11.4f));
    REQUIRE(matD[1][0] == Approx(10.1f));
    REQUIRE(matD[1][1] == Approx(10.7f));
    REQUIRE(matD[1][2] == Approx(8.1f));
    REQUIRE(matD[2][0] == Approx(10.6f));
    REQUIRE(matD[2][1] == Approx(6.8f));
    REQUIRE(matD[2][2] == Approx(3.7f));
    
    Matrix3 matE = 2.4f + matA;
    REQUIRE(matE[0][0] == Approx(6.3f));
    REQUIRE(matE[0][1] == Approx(4.5f));
    REQUIRE(matE[0][2] == Approx(11.4f));
    REQUIRE(matE[1][0] == Approx(10.1f));
    REQUIRE(matE[1][1] == Approx(10.7f));
    REQUIRE(matE[1][2] == Approx(8.1f));
    REQUIRE(matE[2][0] == Approx(10.6f));
    REQUIRE(matE[2][1] == Approx(6.8f));
    REQUIRE(matE[2][2] == Approx(3.7f));
  }
  
  SECTION("Subtraction")
  {
    Matrix3 matC = matA - matB;
    REQUIRE(matC[0][0] == Approx(1.8f));
    REQUIRE(matC[0][1] == Approx(-3.4f));
    REQUIRE(matC[0][2] == Approx(-0.2f));
    REQUIRE(matC[1][0] == Approx(0.7f));
    REQUIRE(matC[1][1] == Approx(6.1f));
    REQUIRE(matC[1][2] == Approx(4.2f));
    REQUIRE(matC[2][0] == Approx(3.5f));
    REQUIRE(matC[2][1] == Approx(1.4f));
    REQUIRE(matC[2][2] == Approx(-0.8f));
    
    Matrix3 matD = matA - 2.4f;
    REQUIRE(matD[0][0] == Approx(1.5f));
    REQUIRE(matD[0][1] == Approx(-0.3f));
    REQUIRE(matD[0][2] == Approx(6.6f));
    REQUIRE(matD[1][0] == Approx(5.3f));
    REQUIRE(matD[1][1] == Approx(5.9f));
    REQUIRE(matD[1][2] == Approx(3.3f));
    REQUIRE(matD[2][0] == Approx(5.8f));
    REQUIRE(matD[2][1] == Approx(2.0f));
    REQUIRE(matD[2][2] == Approx(-1.1));
    
    Matrix3 matE = 2.4f - matA;
    REQUIRE(matE[0][0] == Approx(-1.5f));
    REQUIRE(matE[0][1] == Approx(0.3f));
    REQUIRE(matE[0][2] == Approx(-6.6f));
    REQUIRE(matE[1][0] == Approx(-5.3f));
    REQUIRE(matE[1][1] == Approx(-5.9f));
    REQUIRE(matE[1][2] == Approx(-3.3f));
    REQUIRE(matE[2][0] == Approx(-5.8f));
    REQUIRE(matE[2][1] == Approx(-2.0f));
    REQUIRE(matE[2][2] == Approx(1.1f));
  }
  
  SECTION("Multiplication")
  {
    Matrix3 matC = matA * matB;
    REQUIRE(matC[0][0] == Approx(65.19f));
    REQUIRE(matC[0][1] == Approx(53.07f));
    REQUIRE(matC[0][2] == Approx(57.93f));
    REQUIRE(matC[1][0] == Approx(101.06f));
    REQUIRE(matC[1][1] == Approx(77.71f));
    REQUIRE(matC[1][2] == Approx(95.26f));
    REQUIRE(matC[2][0] == Approx(54.13f));
    REQUIRE(matC[2][1] == Approx(58.68f));
    REQUIRE(matC[2][2] == Approx(84.77f));
    
    Matrix3 matD = matA * 2.4f;
    REQUIRE(matD[0][0] == Approx(9.36f));
    REQUIRE(matD[0][1] == Approx(5.04f));
    REQUIRE(matD[0][2] == Approx(21.6f));
    REQUIRE(matD[1][0] == Approx(18.48f));
    REQUIRE(matD[1][1] == Approx(19.92f));
    REQUIRE(matD[1][2] == Approx(13.68f));
    REQUIRE(matD[2][0] == Approx(19.68f));
    REQUIRE(matD[2][1] == Approx(10.56f));
    REQUIRE(matD[2][2] == Approx(3.12f));
    
    Matrix3 matE = 2.4f * matA;
    REQUIRE(matE[0][0] == Approx(9.36f));
    REQUIRE(matE[0][1] == Approx(5.04f));
    REQUIRE(matE[0][2] == Approx(21.6f));
    REQUIRE(matE[1][0] == Approx(18.48f));
    REQUIRE(matE[1][1] == Approx(19.92f));
    REQUIRE(matE[1][2] == Approx(13.68f));
    REQUIRE(matE[2][0] == Approx(19.68f));
    REQUIRE(matE[2][1] == Approx(10.56f));
    REQUIRE(matE[2][2] == Approx(3.12f));
  }
  
  SECTION("Addition Assignment")
  {
    Matrix3 matC(matA);
    matC += 2.0f;
    REQUIRE(matC[0][0] == Approx(5.9f));
    REQUIRE(matC[0][1] == Approx(4.1f));
    REQUIRE(matC[0][2] == Approx(11.0f));
    REQUIRE(matC[1][0] == Approx(9.7f));
    REQUIRE(matC[1][1] == Approx(10.3f));
    REQUIRE(matC[1][2] == Approx(7.7f));
    REQUIRE(matC[2][0] == Approx(10.2f));
    REQUIRE(matC[2][1] == Approx(6.4f));
    REQUIRE(matC[2][2] == Approx(3.3f));
  }
  
  SECTION("Subtraction Assignment")
  {
    Matrix3 matC(matA);
    matC -= 2.0f;
    REQUIRE(matC[0][0] == Approx(1.9f));
    REQUIRE(matC[0][1] == Approx(0.1f));
    REQUIRE(matC[0][2] == Approx(7.0f));
    REQUIRE(matC[1][0] == Approx(5.7f));
    REQUIRE(matC[1][1] == Approx(6.3f));
    REQUIRE(matC[1][2] == Approx(3.7f));
    REQUIRE(matC[2][0] == Approx(6.2f));
    REQUIRE(matC[2][1] == Approx(2.4f));
    REQUIRE(matC[2][2] == Approx(-0.7f));
  }
  
  SECTION("Mulitplication Assignment")
  {
    Matrix3 matC(matA);
    matC *= 2.0f;
    REQUIRE(matC[0][0] == Approx(7.8f));
    REQUIRE(matC[0][1] == Approx(4.2f));
    REQUIRE(matC[0][2] == Approx(18.0f));
    REQUIRE(matC[1][0] == Approx(15.4f));
    REQUIRE(matC[1][1] == Approx(16.6f));
    REQUIRE(matC[1][2] == Approx(11.4f));
    REQUIRE(matC[2][0] == Approx(16.4f));
    REQUIRE(matC[2][1] == Approx(8.8f));
    REQUIRE(matC[2][2] == Approx(2.6f));
  }
  
  SECTION("Vector Multiplication")
  {
    Vector3 vec(4, 3, 6);
    Vector3 result = matA * vec;
    REQUIRE(result[0] == Approx(75.9f));
    REQUIRE(result[1] == Approx(89.9f));
    REQUIRE(result[2] == Approx(53.8f));
  }
  
  SECTION("Determinate")
  {
    REQUIRE(matA.Determinate() == Approx(-286.218f));
  }
  
  SECTION("Inverse")
  {
    Matrix3 matC = matA.Inverse();
    REQUIRE(matC[0][0] == Approx(0.049927f));
    REQUIRE(matC[0][1] == Approx(-0.128818f));
    REQUIRE(matC[0][2] == Approx(0.219169f));
    REQUIRE(matC[1][0] == Approx(-0.128329f));
    REQUIRE(matC[1][1] == Approx(0.240132f));
    REQUIRE(matC[1][2] == Approx(-0.164455f));
    REQUIRE(matC[2][0] == Approx(0.119419f));
    REQUIRE(matC[2][1] == Approx(-0.000210f).margin(0.000001f));
    REQUIRE(matC[2][2] == Approx(-0.056600f));
  }
  
  SECTION("Transpose")
  {
    Matrix3 matC = matA.Transpose();
    REQUIRE(matC[0][0] == matA[0][0]);
    REQUIRE(matC[0][1] == matA[1][0]);
    REQUIRE(matC[0][2] == matA[2][0]);
    REQUIRE(matC[1][0] == matA[0][1]);
    REQUIRE(matC[1][1] == matA[1][1]);
    REQUIRE(matC[1][2] == matA[2][1]);
    REQUIRE(matC[2][0] == matA[0][2]);
    REQUIRE(matC[2][1] == matA[1][2]);
    REQUIRE(matC[2][2] == matA[2][2]);
  }
}

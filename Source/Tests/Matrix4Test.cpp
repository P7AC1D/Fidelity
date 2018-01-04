#include "catch.hpp"

#include "../Engine/Maths/Degree.hpp"
#include "../Engine/Maths/Matrix3.hpp"
#include "../Engine/Maths/Matrix4.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Radian.hpp"
#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Maths/Vector4.hpp"

TEST_CASE("Matrix4 Constructors and Accessors")
{
  SECTION("Zero")
  {
    Matrix4 mat = Matrix4::Zero;
    REQUIRE(mat[0][0] == 0.0f);
    REQUIRE(mat[0][1] == 0.0f);
    REQUIRE(mat[0][2] == 0.0f);
    REQUIRE(mat[0][3] == 0.0f);
    REQUIRE(mat[1][0] == 0.0f);
    REQUIRE(mat[1][1] == 0.0f);
    REQUIRE(mat[1][2] == 0.0f);
    REQUIRE(mat[1][3] == 0.0f);
    REQUIRE(mat[2][0] == 0.0f);
    REQUIRE(mat[2][1] == 0.0f);
    REQUIRE(mat[2][2] == 0.0f);
    REQUIRE(mat[2][3] == 0.0f);
    REQUIRE(mat[3][0] == 0.0f);
    REQUIRE(mat[3][1] == 0.0f);
    REQUIRE(mat[3][2] == 0.0f);
    REQUIRE(mat[3][3] == 0.0f);
  }
  
  SECTION("Identity")
  {
    Matrix4 mat = Matrix4::Identity;
    REQUIRE(mat[0][0] == 1.0f);
    REQUIRE(mat[0][1] == 0.0f);
    REQUIRE(mat[0][2] == 0.0f);
    REQUIRE(mat[0][3] == 0.0f);
    REQUIRE(mat[1][0] == 0.0f);
    REQUIRE(mat[1][1] == 1.0f);
    REQUIRE(mat[1][2] == 0.0f);
    REQUIRE(mat[1][3] == 0.0f);
    REQUIRE(mat[2][0] == 0.0f);
    REQUIRE(mat[2][1] == 0.0f);
    REQUIRE(mat[2][2] == 1.0f);
    REQUIRE(mat[2][3] == 0.0f);
    REQUIRE(mat[3][0] == 0.0f);
    REQUIRE(mat[3][1] == 0.0f);
    REQUIRE(mat[3][2] == 0.0f);
    REQUIRE(mat[3][3] == 1.0f);
  }
  
  SECTION("Full Constructor")
  {
    float32 m00 = 4.0f, m01 = 2.4f, m02 = 6.2f, m03 = 8.0f;
    float32 m10 = 2.9f, m11 = 1.6f, m12 = 4.4f, m13 = 5.1f;
    float32 m20 = 9.2f, m21 = 4.1f, m22 = 7.5f, m23 = 0.2f;
    float32 m30 = 9.4f, m31 = 1.6f, m32 = 2.4f, m33 = 0.9f;
    Matrix4 mat(m00, m01, m02, m03,
                m10, m11, m12, m13,
                m20, m21, m22, m23,
                m30, m31, m32, m33);
    REQUIRE(mat[0][0] == m00);
    REQUIRE(mat[0][1] == m01);
    REQUIRE(mat[0][2] == m02);
    REQUIRE(mat[0][3] == m03);
    REQUIRE(mat[1][0] == m10);
    REQUIRE(mat[1][1] == m11);
    REQUIRE(mat[1][2] == m12);
    REQUIRE(mat[1][3] == m13);
    REQUIRE(mat[2][0] == m20);
    REQUIRE(mat[2][1] == m21);
    REQUIRE(mat[2][2] == m22);
    REQUIRE(mat[2][3] == m23);
    REQUIRE(mat[3][0] == m30);
    REQUIRE(mat[3][1] == m31);
    REQUIRE(mat[3][2] == m32);
    REQUIRE(mat[3][3] == m33);
  }
  
  SECTION("Copy Constructor")
  {
    float32 m00 = 4.0f, m01 = 2.4f, m02 = 6.2f, m03 = 8.0f;
    float32 m10 = 2.9f, m11 = 1.6f, m12 = 4.4f, m13 = 5.1f;
    float32 m20 = 9.2f, m21 = 4.1f, m22 = 7.5f, m23 = 0.2f;
    float32 m30 = 9.4f, m31 = 1.6f, m32 = 2.4f, m33 = 0.9f;
    Matrix4 matA(m00, m01, m02, m03,
                 m10, m11, m12, m13,
                 m20, m21, m22, m23,
                 m30, m31, m32, m33);
    Matrix4 matB(matA);
    REQUIRE(matB[0][0] == m00);
    REQUIRE(matB[0][1] == m01);
    REQUIRE(matB[0][2] == m02);
    REQUIRE(matB[0][3] == m03);
    REQUIRE(matB[1][0] == m10);
    REQUIRE(matB[1][1] == m11);
    REQUIRE(matB[1][2] == m12);
    REQUIRE(matB[1][3] == m13);
    REQUIRE(matB[2][0] == m20);
    REQUIRE(matB[2][1] == m21);
    REQUIRE(matB[2][2] == m22);
    REQUIRE(matB[2][3] == m23);
    REQUIRE(matB[3][0] == m30);
    REQUIRE(matB[3][1] == m31);
    REQUIRE(matB[3][2] == m32);
    REQUIRE(matB[3][3] == m33);
  }
  
  SECTION("Matrix3 Copy Constructor")
  {
    float32 m00 = 4.0f, m01 = 2.4f, m02 = 6.2f;
    float32 m10 = 2.9f, m11 = 1.6f, m12 = 4.4f;
    float32 m20 = 9.2f, m21 = 4.1f, m22 = 7.5f;
    Matrix3 matA(m00, m01, m02,
                 m10, m11, m12,
                 m20, m21, m22);
    Matrix4 matB(matA);
    REQUIRE(matB[0][0] == m00);
    REQUIRE(matB[0][1] == m01);
    REQUIRE(matB[0][2] == m02);
    REQUIRE(matB[0][3] == 0.0f);
    REQUIRE(matB[1][0] == m10);
    REQUIRE(matB[1][1] == m11);
    REQUIRE(matB[1][2] == m12);
    REQUIRE(matB[1][3] == 0.0f);
    REQUIRE(matB[2][0] == m20);
    REQUIRE(matB[2][1] == m21);
    REQUIRE(matB[2][2] == m22);
    REQUIRE(matB[2][3] == 0.0f);
    REQUIRE(matB[3][0] == 0.0f);
    REQUIRE(matB[3][1] == 0.0f);
    REQUIRE(matB[3][2] == 0.0f);
    REQUIRE(matB[3][3] == 1.0f);
  }
  
  SECTION("Quaternion Copy Constructor")
  {
    Quaternion qat(2.2f, 4.4f, 2.0f, 6.2f);
    qat.Normalize();
    Matrix4 matA(qat);
    
    REQUIRE(matA[0][0] == Approx(-0.2737095f));
    REQUIRE(matA[0][1] == Approx(-0.1452581f));
    REQUIRE(matA[0][2] == Approx(0.9507803f));
    REQUIRE(matA[0][3] == 0.0f);
    REQUIRE(matA[1][0] == Approx(0.6734694f));
    REQUIRE(matA[1][1] == Approx(-0.7346939f));
    REQUIRE(matA[1][2] == Approx(0.0816327f));
    REQUIRE(matA[1][3] == 0.0f);
    REQUIRE(matA[2][0] == Approx(0.6866747f));
    REQUIRE(matA[2][1] == Approx(0.6626651f));
    REQUIRE(matA[2][2] == Approx(0.2989196f));
    REQUIRE(matA[2][3] == 0.0f);
    REQUIRE(matA[3][0] == 0.0f);
    REQUIRE(matA[3][1] == 0.0f);
    REQUIRE(matA[3][2] == 0.0f);
    REQUIRE(matA[3][3] == 1.0f);
  }
}

TEST_CASE("Matrix4 Mathematical Operators")
{
  Matrix4 matA(4.3f,  7.9f,  2.4f, -2.5f,
               6.1f, -3.9f,  7.8f,  1.1f,
               3.1f,  5.5f, -9.2f,  0.4f,
               0.5f,  4.2f,  9.4f,  8.3f);
  Matrix4 matB(-3.5f, 3.5f,  8.9f,  0.8f,
                3.2f, 2.7f,  6.5f,  2.5f,
                9.9f, 11.5f, -7.2f, 8.1f,
                6.2f, 3.3f,  4.2f,  4.4f);
  
  SECTION("Matrix Addition")
  {
    Matrix4 matC = matA + matB;
    REQUIRE(matC[0][0] == Approx(0.8f));
    REQUIRE(matC[0][1] == Approx(11.4f));
    REQUIRE(matC[0][2] == Approx(11.3f));
    REQUIRE(matC[0][3] == Approx(-1.7f));
    REQUIRE(matC[1][0] == Approx(9.3f));
    REQUIRE(matC[1][1] == Approx(-1.2f));
    REQUIRE(matC[1][2] == Approx(14.3f));
    REQUIRE(matC[1][3] == Approx(3.6f));
    REQUIRE(matC[2][0] == Approx(13.0f));
    REQUIRE(matC[2][1] == Approx(17.0f));
    REQUIRE(matC[2][2] == Approx(-16.4f));
    REQUIRE(matC[2][3] == Approx(8.5f));
    REQUIRE(matC[3][0] == Approx(6.7f));
    REQUIRE(matC[3][1] == Approx(7.5f));
    REQUIRE(matC[3][2] == Approx(13.6f));
    REQUIRE(matC[3][3] == Approx(12.7f));
  }
  
  SECTION("Scalar Addition")
  {
    Matrix4 matC = matA + 1.0f;
    REQUIRE(matC[0][0] == Approx(5.3f));
    REQUIRE(matC[0][1] == Approx(8.9f));
    REQUIRE(matC[0][2] == Approx(3.4f));
    REQUIRE(matC[0][3] == Approx(-1.5f));
    REQUIRE(matC[1][0] == Approx(7.1f));
    REQUIRE(matC[1][1] == Approx(-2.9f));
    REQUIRE(matC[1][2] == Approx(8.8f));
    REQUIRE(matC[1][3] == Approx(2.1f));
    REQUIRE(matC[2][0] == Approx(4.1f));
    REQUIRE(matC[2][1] == Approx(6.5f));
    REQUIRE(matC[2][2] == Approx(-8.2f));
    REQUIRE(matC[2][3] == Approx(1.4f));
    REQUIRE(matC[3][0] == Approx(1.5f));
    REQUIRE(matC[3][1] == Approx(5.2f));
    REQUIRE(matC[3][2] == Approx(10.4f));
    REQUIRE(matC[3][3] == Approx(9.3f));
    
    Matrix4 matD = 1.0f + matA;
    REQUIRE(matD[0][0] == Approx(5.3f));
    REQUIRE(matD[0][1] == Approx(8.9f));
    REQUIRE(matD[0][2] == Approx(3.4f));
    REQUIRE(matD[0][3] == Approx(-1.5f));
    REQUIRE(matD[1][0] == Approx(7.1f));
    REQUIRE(matD[1][1] == Approx(-2.9f));
    REQUIRE(matD[1][2] == Approx(8.8f));
    REQUIRE(matD[1][3] == Approx(2.1f));
    REQUIRE(matD[2][0] == Approx(4.1f));
    REQUIRE(matD[2][1] == Approx(6.5f));
    REQUIRE(matD[2][2] == Approx(-8.2f));
    REQUIRE(matD[2][3] == Approx(1.4f));
    REQUIRE(matD[3][0] == Approx(1.5f));
    REQUIRE(matD[3][1] == Approx(5.2f));
    REQUIRE(matD[3][2] == Approx(10.4f));
    REQUIRE(matD[3][3] == Approx(9.3f));
  }
  
  SECTION("Matrix Subtraction")
  {
    Matrix4 matC = matA - matB;
    REQUIRE(matC[0][0] == Approx(7.8f));
    REQUIRE(matC[0][1] == Approx(4.4f));
    REQUIRE(matC[0][2] == Approx(-6.5f));
    REQUIRE(matC[0][3] == Approx(-3.3f));
    REQUIRE(matC[1][0] == Approx(2.9f));
    REQUIRE(matC[1][1] == Approx(-6.6f));
    REQUIRE(matC[1][2] == Approx(1.3f));
    REQUIRE(matC[1][3] == Approx(-1.4f));
    REQUIRE(matC[2][0] == Approx(-6.8f));
    REQUIRE(matC[2][1] == Approx(-6.0f));
    REQUIRE(matC[2][2] == Approx(-2.0f));
    REQUIRE(matC[2][3] == Approx(-7.7f));
    REQUIRE(matC[3][0] == Approx(-5.7f));
    REQUIRE(matC[3][1] == Approx(0.9f));
    REQUIRE(matC[3][2] == Approx(5.2f));
    REQUIRE(matC[3][3] == Approx(3.9f));
  }
  
  SECTION("Scalar Subtraction")
  {
    Matrix4 matC = matA - 1.0f;
    REQUIRE(matC[0][0] == Approx(3.3f));
    REQUIRE(matC[0][1] == Approx(6.9f));
    REQUIRE(matC[0][2] == Approx(1.4f));
    REQUIRE(matC[0][3] == Approx(-3.5f));
    REQUIRE(matC[1][0] == Approx(5.1f));
    REQUIRE(matC[1][1] == Approx(-4.9f));
    REQUIRE(matC[1][2] == Approx(6.8f));
    REQUIRE(matC[1][3] == Approx(0.1f));
    REQUIRE(matC[2][0] == Approx(2.1f));
    REQUIRE(matC[2][1] == Approx(4.5f));
    REQUIRE(matC[2][2] == Approx(-10.2f));
    REQUIRE(matC[2][3] == Approx(-0.6f));
    REQUIRE(matC[3][0] == Approx(-0.5f));
    REQUIRE(matC[3][1] == Approx(3.2f));
    REQUIRE(matC[3][2] == Approx(8.4f));
    REQUIRE(matC[3][3] == Approx(7.3f));
    
    Matrix4 matD = 1.0f - matA;
    REQUIRE(matD[0][0] == Approx(-3.3f));
    REQUIRE(matD[0][1] == Approx(-6.9f));
    REQUIRE(matD[0][2] == Approx(-1.4f));
    REQUIRE(matD[0][3] == Approx(3.5f));
    REQUIRE(matD[1][0] == Approx(-5.1f));
    REQUIRE(matD[1][1] == Approx(4.9f));
    REQUIRE(matD[1][2] == Approx(-6.8f));
    REQUIRE(matD[1][3] == Approx(-0.1f));
    REQUIRE(matD[2][0] == Approx(-2.1f));
    REQUIRE(matD[2][1] == Approx(-4.5f));
    REQUIRE(matD[2][2] == Approx(10.2f));
    REQUIRE(matD[2][3] == Approx(0.6f));
    REQUIRE(matD[3][0] == Approx(0.5f));
    REQUIRE(matD[3][1] == Approx(-3.2f));
    REQUIRE(matD[3][2] == Approx(-8.4f));
    REQUIRE(matD[3][3] == Approx(-7.3f));
  }
  
  SECTION("Matrix Multiplication")
  {
    Matrix4 matC = matA * matB;
    REQUIRE(matC[0][0] == Approx(18.49f));
    REQUIRE(matC[0][1] == Approx(55.73f));
    REQUIRE(matC[0][2] == Approx(61.84f));
    REQUIRE(matC[0][3] == Approx(31.63f));
    REQUIRE(matC[1][0] == Approx(50.21f));
    REQUIRE(matC[1][1] == Approx(104.15f));
    REQUIRE(matC[1][2] == Approx(-22.6f));
    REQUIRE(matC[1][3] == Approx(63.15f));
    REQUIRE(matC[2][0] == Approx(-81.85f));
    REQUIRE(matC[2][1] == Approx(-78.78f));
    REQUIRE(matC[2][2] == Approx(131.26f));
    REQUIRE(matC[2][3] == Approx(-56.53f));
    REQUIRE(matC[3][0] == Approx(156.21f));
    REQUIRE(matC[3][1] == Approx(148.58f));
    REQUIRE(matC[3][2] == Approx(-1.07f));
    REQUIRE(matC[3][3] == Approx(123.56f));
  }
  
  SECTION("Scalar Multiplication")
  {
    Matrix4 matC = matA * 2.0f;
    REQUIRE(matC[0][0] == Approx(8.6f));
    REQUIRE(matC[0][1] == Approx(15.8f));
    REQUIRE(matC[0][2] == Approx(4.8f));
    REQUIRE(matC[0][3] == Approx(-5.0f));
    REQUIRE(matC[1][0] == Approx(12.2f));
    REQUIRE(matC[1][1] == Approx(-7.8f));
    REQUIRE(matC[1][2] == Approx(15.6f));
    REQUIRE(matC[1][3] == Approx(2.2f));
    REQUIRE(matC[2][0] == Approx(6.2f));
    REQUIRE(matC[2][1] == Approx(11.0f));
    REQUIRE(matC[2][2] == Approx(-18.4f));
    REQUIRE(matC[2][3] == Approx(0.8f));
    REQUIRE(matC[3][0] == Approx(1.0f));
    REQUIRE(matC[3][1] == Approx(8.4f));
    REQUIRE(matC[3][2] == Approx(18.8f));
    REQUIRE(matC[3][3] == Approx(16.6f));
    
    Matrix4 matD = 2.0f * matA;
    REQUIRE(matD[0][0] == Approx(8.6f));
    REQUIRE(matD[0][1] == Approx(15.8f));
    REQUIRE(matD[0][2] == Approx(4.8f));
    REQUIRE(matD[0][3] == Approx(-5.0f));
    REQUIRE(matD[1][0] == Approx(12.2f));
    REQUIRE(matD[1][1] == Approx(-7.8f));
    REQUIRE(matD[1][2] == Approx(15.6f));
    REQUIRE(matD[1][3] == Approx(2.2f));
    REQUIRE(matD[2][0] == Approx(6.2f));
    REQUIRE(matD[2][1] == Approx(11.0f));
    REQUIRE(matD[2][2] == Approx(-18.4f));
    REQUIRE(matD[2][3] == Approx(0.8f));
    REQUIRE(matD[3][0] == Approx(1.0f));
    REQUIRE(matD[3][1] == Approx(8.4f));
    REQUIRE(matD[3][2] == Approx(18.8f));
    REQUIRE(matD[3][3] == Approx(16.6f));
  }
  
  SECTION("Vector4 Mulitplication")
  {
    Vector4 vec = matA * Vector4(4.0f, -3.5f, 8.0f, -2.2f);
    REQUIRE(vec[0] == Approx(14.249999f));
    REQUIRE(vec[1] == Approx(98.029999f));
    REQUIRE(vec[2] == Approx(-81.329999f));
    REQUIRE(vec[3] == Approx(44.239999f));
    
    Vector4 vecB = Vector4(4.0f, -3.5f, 8.0f, -2.2f) * matA;
    REQUIRE(vecB[0] == Approx(14.249999f));
    REQUIRE(vecB[1] == Approx(98.029999f));
    REQUIRE(vecB[2] == Approx(-81.329999f));
    REQUIRE(vecB[3] == Approx(44.239999f));
  }
  
  SECTION("Equality Operators")
  {
    REQUIRE(matA == matA);
    REQUIRE(matA != matB);
    REQUIRE(matB != matA);
  }
}

TEST_CASE("Matrix4 Transformation")
{
  Vector3 scale(4, 3, 5);
  Vector3 position(5, 3, 1);
  Quaternion orientation(Degree(90), Degree(0), Degree(0));

  SECTION("Rotation")
  {
    Matrix4 rotResult(Matrix4::Rotation(orientation));
    Matrix4 rotExpect(Matrix4::Zero);
    rotExpect[0][0] = 1.0f;
    rotExpect[1][2] = -1.0f;
    rotExpect[2][1] = 1.0f;
    rotExpect[3][3] = 1.0f;

    REQUIRE(rotResult[0][0] == Approx(1.0f).margin(0.00001f));
    REQUIRE(rotResult[0][1] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[0][2] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[0][3] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[1][0] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[1][1] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[1][2] == Approx(-1.0f).margin(0.00001f));
    REQUIRE(rotResult[1][3] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[2][0] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[2][1] == Approx(1.0f).margin(0.00001f));
    REQUIRE(rotResult[2][2] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[2][3] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[3][0] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[3][1] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[3][2] == Approx(0.0f).margin(0.00001f));
    REQUIRE(rotResult[3][3] == Approx(1.0f).margin(0.00001f));
  }
  
  SECTION("Translation")
  {
    Matrix4 transResult(Matrix4::Translation(position));
    Matrix4 transExpect(Matrix4::Identity);
    transExpect[0][3] = position[0];
    transExpect[1][3] = position[1];
    transExpect[2][3] = position[2];

    REQUIRE(transResult == transExpect);
  }

  SECTION("Scaling")
  {
    Matrix4 scaleResult(Matrix4::Scaling(scale));
    Matrix4 scaleExpect(Matrix4::Identity);
    scaleExpect[0][0] = scale[0];
    scaleExpect[1][1] = scale[1];
    scaleExpect[2][2] = scale[2];

    REQUIRE(scaleResult == scaleExpect);
  }
}

TEST_CASE("Matrix4 Determinate")
{
  Matrix4 matA(4.3f,  7.9f,  2.4f, -2.5f,
               6.1f, -3.9f,  7.8f,  1.1f,
               3.1f,  5.5f, -9.2f,  0.4f,
               0.5f,  4.2f,  9.4f,  8.3f);
  REQUIRE(matA.Determinate() == Approx(8253.817000f));
}

TEST_CASE("Matrix4 Inverse")
{
  Matrix4 matA(4.3f,  7.9f,  2.4f, -2.5f,
               6.1f, -3.9f,  7.8f,  1.1f,
               3.1f,  5.5f, -9.2f,  0.4f,
               0.5f,  4.2f,  9.4f,  8.3f);
  Matrix4 matC = matA.Inverse();
  REQUIRE(matC[0][0] == Approx(0.00834f).margin(0.00001f));
  REQUIRE(matC[0][1] == Approx(0.116834f));
  REQUIRE(matC[0][2] == Approx(0.083850f));
  REQUIRE(matC[0][3] == Approx(-0.017012f).margin(0.00001f));
  REQUIRE(matC[1][0] == Approx(0.078843f));
  REQUIRE(matC[1][1] == Approx(-0.059383f));
  REQUIRE(matC[1][2] == Approx(0.002408f).margin(0.00001f));
  REQUIRE(matC[1][3] == Approx(0.031502f));
  REQUIRE(matC[2][0] == Approx(0.045928f));
  REQUIRE(matC[2][1] == Approx(0.004639f).margin(0.00001f));
  REQUIRE(matC[2][2] == Approx(-0.075555f));
  REQUIRE(matC[2][3] == Approx(0.016860f));
  REQUIRE(matC[3][0] == Approx(-0.092414f));
  REQUIRE(matC[3][1] == Approx(0.017757f));
  REQUIRE(matC[3][2] == Approx(0.079298f));
  REQUIRE(matC[3][3] == Approx(0.086472f));
}

TEST_CASE("Matrix4 Transpose")
{
  Matrix4 matC(4.3f,  7.9f,  2.4f, -2.5f,
               6.1f, -3.9f,  7.8f,  1.1f,
               3.1f,  5.5f, -9.2f,  0.4f,
               0.5f,  4.2f,  9.4f,  8.3f);
  Matrix4 matD = matC.Transpose();
  REQUIRE(matD[0][0] == matC[0][0]);
  REQUIRE(matD[0][1] == matC[1][0]);
  REQUIRE(matD[0][2] == matC[2][0]);
  REQUIRE(matD[0][3] == matC[3][0]);
  REQUIRE(matD[1][0] == matC[0][1]);
  REQUIRE(matD[1][1] == matC[1][1]);
  REQUIRE(matD[1][2] == matC[2][1]);
  REQUIRE(matD[1][3] == matC[3][1]);
  REQUIRE(matD[2][0] == matC[0][2]);
  REQUIRE(matD[2][1] == matC[1][2]);
  REQUIRE(matD[2][2] == matC[2][2]);
  REQUIRE(matD[2][3] == matC[3][2]);
  REQUIRE(matD[3][0] == matC[0][3]);
  REQUIRE(matD[3][1] == matC[1][3]);
  REQUIRE(matD[3][2] == matC[2][3]);
  REQUIRE(matD[3][3] == matC[3][3]);
}

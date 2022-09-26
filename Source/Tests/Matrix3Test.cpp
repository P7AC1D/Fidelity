#include "catch.hpp"

#include <glm/gtx/quaternion.hpp>

#include "../Engine/Maths/Matrix3.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Vector3.hpp"

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

    glm::quat resultQuat(2.2f, 4.4f, 2.0f, 6.2f);
    resultQuat = glm::normalize(resultQuat);
    glm::mat3 resultMat = glm::toMat3(resultQuat);

    REQUIRE(matA[0][0] == Approx(resultMat[0][0]));
    REQUIRE(matA[0][1] == Approx(resultMat[0][1]));
    REQUIRE(matA[0][2] == Approx(resultMat[0][2]));
    REQUIRE(matA[1][0] == Approx(resultMat[1][0]));
    REQUIRE(matA[1][1] == Approx(resultMat[1][1]));
    REQUIRE(matA[1][2] == Approx(resultMat[1][2]));
    REQUIRE(matA[2][0] == Approx(resultMat[2][0]));
    REQUIRE(matA[2][1] == Approx(resultMat[2][1]));
    REQUIRE(matA[2][2] == Approx(resultMat[2][2]));
  }
}

TEST_CASE("Matrix3 Quaternion Equality Operators")
{
  Matrix3 matA(3.9f, 2.1f, 9.0f,
               7.7f, 8.3f, 5.7f,
               8.2f, 4.4f, 1.3f);
  REQUIRE(matA == matA);

  Matrix3 matB(2.2f, 9.4f, 3.0f,
               5.4f, 5.2f, 2.78f,
               6.4f, 6.8f, 2.5f);
  REQUIRE(matA != matB);
  REQUIRE(matB != matA);
}

TEST_CASE("Matrix3 Mathematical Operators")
{
  Matrix3 matA(3.9f, 2.1f, 9.0f,
               7.7f, 8.3f, 5.7f,
               8.2f, 4.4f, 1.3f);
  Matrix3 matB(2.1f, 5.5f, 9.2f,
               7.0f, 2.2f, 1.5f,
               4.7f, 3.0f, 2.1f);

  glm::mat3 expectedA(3.9f, 2.1f, 9.0f,
                      7.7f, 8.3f, 5.7f,
                      8.2f, 4.4f, 1.3f);
  glm::mat3 expectedB(2.1f, 5.5f, 9.2f,
                      7.0f, 2.2f, 1.5f,
                      4.7f, 3.0f, 2.1f);

  SECTION("Addition")
  {
    Matrix3 matC = matA + matB;
    glm::mat3 expectedC = expectedA + expectedB;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);

    Matrix3 matD = matA + 2.4f;
    glm::mat3 expectedD = expectedA + 2.4f;
    REQUIRE(matD[0][0] == expectedD[0][0]);
    REQUIRE(matD[0][1] == expectedD[0][1]);
    REQUIRE(matD[0][2] == expectedD[0][2]);
    REQUIRE(matD[1][0] == expectedD[1][0]);
    REQUIRE(matD[1][1] == expectedD[1][1]);
    REQUIRE(matD[1][2] == expectedD[1][2]);
    REQUIRE(matD[2][0] == expectedD[2][0]);
    REQUIRE(matD[2][1] == expectedD[2][1]);
    REQUIRE(matD[2][2] == expectedD[2][2]);

    Matrix3 matE = 2.4f + matA;
    glm::mat3 expectedE = 2.4f + expectedA;
    REQUIRE(matE[0][0] == expectedE[0][0]);
    REQUIRE(matE[0][1] == expectedE[0][1]);
    REQUIRE(matE[0][2] == expectedE[0][2]);
    REQUIRE(matE[1][0] == expectedE[1][0]);
    REQUIRE(matE[1][1] == expectedE[1][1]);
    REQUIRE(matE[1][2] == expectedE[1][2]);
    REQUIRE(matE[2][0] == expectedE[2][0]);
    REQUIRE(matE[2][1] == expectedE[2][1]);
    REQUIRE(matE[2][2] == expectedE[2][2]);
  }

  SECTION("Subtraction")
  {
    Matrix3 matC = matA - matB;
    glm::mat3 expectedC = expectedA + -expectedB;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);

    Matrix3 matD = matA - 2.4f;
    glm::mat3 expectedD = expectedA - 2.4f;
    REQUIRE(matD[0][0] == expectedD[0][0]);
    REQUIRE(matD[0][1] == expectedD[0][1]);
    REQUIRE(matD[0][2] == expectedD[0][2]);
    REQUIRE(matD[1][0] == expectedD[1][0]);
    REQUIRE(matD[1][1] == expectedD[1][1]);
    REQUIRE(matD[1][2] == expectedD[1][2]);
    REQUIRE(matD[2][0] == expectedD[2][0]);
    REQUIRE(matD[2][1] == expectedD[2][1]);
    REQUIRE(matD[2][2] == expectedD[2][2]);

    Matrix3 matE = 2.4f - matA;
    glm::mat3 expectedE = 2.4f - expectedA;
    REQUIRE(matE[0][0] == expectedE[0][0]);
    REQUIRE(matE[0][1] == expectedE[0][1]);
    REQUIRE(matE[0][2] == expectedE[0][2]);
    REQUIRE(matE[1][0] == expectedE[1][0]);
    REQUIRE(matE[1][1] == expectedE[1][1]);
    REQUIRE(matE[1][2] == expectedE[1][2]);
    REQUIRE(matE[2][0] == expectedE[2][0]);
    REQUIRE(matE[2][1] == expectedE[2][1]);
    REQUIRE(matE[2][2] == expectedE[2][2]);
  }

  SECTION("Multiplication")
  {
    Matrix3 matC = matA * matB;
    glm::mat3 expectedC = expectedA * expectedB;

    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);

    Matrix3 matD = matA * 2.4f;
    glm::mat3 expectedD = expectedA * 2.4f;
    REQUIRE(matD[0][0] == expectedD[0][0]);
    REQUIRE(matD[0][1] == expectedD[0][1]);
    REQUIRE(matD[0][2] == expectedD[0][2]);
    REQUIRE(matD[1][0] == expectedD[1][0]);
    REQUIRE(matD[1][1] == expectedD[1][1]);
    REQUIRE(matD[1][2] == expectedD[1][2]);
    REQUIRE(matD[2][0] == expectedD[2][0]);
    REQUIRE(matD[2][1] == expectedD[2][1]);
    REQUIRE(matD[2][2] == expectedD[2][2]);

    Matrix3 matE = 2.4f * matA;
    glm::mat3 expectedE = 2.4f * expectedA;
    REQUIRE(matE[0][0] == expectedE[0][0]);
    REQUIRE(matE[0][1] == expectedE[0][1]);
    REQUIRE(matE[0][2] == expectedE[0][2]);
    REQUIRE(matE[1][0] == expectedE[1][0]);
    REQUIRE(matE[1][1] == expectedE[1][1]);
    REQUIRE(matE[1][2] == expectedE[1][2]);
    REQUIRE(matE[2][0] == expectedE[2][0]);
    REQUIRE(matE[2][1] == expectedE[2][1]);
    REQUIRE(matE[2][2] == expectedE[2][2]);
  }

  SECTION("Addition Assignment")
  {
    Matrix3 matC(matA);
    glm::mat3 expectedC(expectedA);
    matC += 2.0f;
    expectedC += 2.0f;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
  }

  SECTION("Subtraction Assignment")
  {
    Matrix3 matC(matA);
    glm::mat3 expectedC(expectedA);
    matC -= 2.0f;
    expectedC -= 2.0f;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
  }

  SECTION("Mulitplication Assignment")
  {
    Matrix3 matC(matA);
    glm::mat3 expectedC(expectedA);
    matC *= 2.0f;
    expectedC *= 2.0f;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
  }

  SECTION("Vector Multiplication")
  {
    Vector3 vec(4, 3, 6);

    Vector3 result = matA * vec;
    glm::vec3 expectedVec = expectedA * glm::vec3(4, 3, 6);
    REQUIRE(result[0] == expectedVec[0]);
    REQUIRE(result[1] == expectedVec[1]);
    REQUIRE(result[2] == expectedVec[2]);
  }

  SECTION("Determinate")
  {
    REQUIRE(matA.Determinate() == glm::determinant(expectedA));
  }

  SECTION("Inverse")
  {
    Matrix3 matC = matA.Inverse();
    glm::mat3 expectedC = glm::inverse(expectedA);
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
  }

  SECTION("Transpose")
  {
    Matrix3 matC = matA.Transpose();
    glm::mat3 expectedC(glm::transpose(expectedA));
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
  }
}

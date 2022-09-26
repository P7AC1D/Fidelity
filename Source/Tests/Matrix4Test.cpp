#include "catch.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

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
    glm::mat4 expected(glm::toMat4(glm::normalize(glm::quat(2.2f, 4.4f, 2.0f, 6.2f))));

    REQUIRE(matA[0][0] == expected[0][0]);
    REQUIRE(matA[0][1] == expected[0][1]);
    REQUIRE(matA[0][2] == expected[0][2]);
    REQUIRE(matA[0][3] == expected[0][3]);
    REQUIRE(matA[1][0] == expected[1][0]);
    REQUIRE(matA[1][1] == expected[1][1]);
    REQUIRE(matA[1][2] == expected[1][2]);
    REQUIRE(matA[1][3] == expected[1][3]);
    REQUIRE(matA[2][0] == expected[2][0]);
    REQUIRE(matA[2][1] == expected[2][1]);
    REQUIRE(matA[2][2] == expected[2][2]);
    REQUIRE(matA[2][3] == expected[2][3]);
    REQUIRE(matA[3][0] == expected[3][0]);
    REQUIRE(matA[3][1] == expected[3][1]);
    REQUIRE(matA[3][2] == expected[3][2]);
    REQUIRE(matA[3][3] == expected[3][3]);
  }
}

TEST_CASE("Matrix4 Mathematical Operators")
{
  Matrix4 matA(4.3f, 7.9f, 2.4f, -2.5f,
               6.1f, -3.9f, 7.8f, 1.1f,
               3.1f, 5.5f, -9.2f, 0.4f,
               0.5f, 4.2f, 9.4f, 8.3f);
  Matrix4 matB(-3.5f, 3.5f, 8.9f, 0.8f,
               3.2f, 2.7f, 6.5f, 2.5f,
               9.9f, 11.5f, -7.2f, 8.1f,
               6.2f, 3.3f, 4.2f, 4.4f);

  glm::mat4 expectedA(4.3f, 7.9f, 2.4f, -2.5f,
                      6.1f, -3.9f, 7.8f, 1.1f,
                      3.1f, 5.5f, -9.2f, 0.4f,
                      0.5f, 4.2f, 9.4f, 8.3f);
  glm::mat4 expectedB(-3.5f, 3.5f, 8.9f, 0.8f,
                      3.2f, 2.7f, 6.5f, 2.5f,
                      9.9f, 11.5f, -7.2f, 8.1f,
                      6.2f, 3.3f, 4.2f, 4.4f);

  SECTION("Matrix Addition")
  {
    Matrix4 matC = matA + matB;
    glm::mat4 expectedC = expectedA + expectedB;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[0][3] == expectedC[0][3]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[1][3] == expectedC[1][3]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
    REQUIRE(matC[2][3] == expectedC[2][3]);
    REQUIRE(matC[3][0] == expectedC[3][0]);
    REQUIRE(matC[3][1] == expectedC[3][1]);
    REQUIRE(matC[3][2] == expectedC[3][2]);
    REQUIRE(matC[3][3] == expectedC[3][3]);
  }

  SECTION("Scalar Addition")
  {
    Matrix4 matC = matA + 1.0f;
    glm::mat4 expectedC = expectedA + 1.0f;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[0][3] == expectedC[0][3]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[1][3] == expectedC[1][3]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
    REQUIRE(matC[2][3] == expectedC[2][3]);
    REQUIRE(matC[3][0] == expectedC[3][0]);
    REQUIRE(matC[3][1] == expectedC[3][1]);
    REQUIRE(matC[3][2] == expectedC[3][2]);
    REQUIRE(matC[3][3] == expectedC[3][3]);

    Matrix4 matD = 1.0f + matA;
    glm::mat4 expectedD = 1.0f + expectedA;
    REQUIRE(matD[0][0] == expectedD[0][0]);
    REQUIRE(matD[0][1] == expectedD[0][1]);
    REQUIRE(matD[0][2] == expectedD[0][2]);
    REQUIRE(matD[0][3] == expectedD[0][3]);
    REQUIRE(matD[1][0] == expectedD[1][0]);
    REQUIRE(matD[1][1] == expectedD[1][1]);
    REQUIRE(matD[1][2] == expectedD[1][2]);
    REQUIRE(matD[1][3] == expectedD[1][3]);
    REQUIRE(matD[2][0] == expectedD[2][0]);
    REQUIRE(matD[2][1] == expectedD[2][1]);
    REQUIRE(matD[2][2] == expectedD[2][2]);
    REQUIRE(matD[2][3] == expectedD[2][3]);
    REQUIRE(matD[3][0] == expectedD[3][0]);
    REQUIRE(matD[3][1] == expectedD[3][1]);
    REQUIRE(matD[3][2] == expectedD[3][2]);
    REQUIRE(matD[3][3] == expectedD[3][3]);
  }

  SECTION("Matrix Subtraction")
  {
    Matrix4 matC = matA - matB;
    glm::mat4 expectedC = expectedA - expectedB;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[0][3] == expectedC[0][3]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[1][3] == expectedC[1][3]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
    REQUIRE(matC[2][3] == expectedC[2][3]);
    REQUIRE(matC[3][0] == expectedC[3][0]);
    REQUIRE(matC[3][1] == expectedC[3][1]);
    REQUIRE(matC[3][2] == expectedC[3][2]);
    REQUIRE(matC[3][3] == expectedC[3][3]);
  }

  SECTION("Scalar Subtraction")
  {
    Matrix4 matC = matA - 1.0f;
    glm::mat4 expectedC = expectedA - 1.0f;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[0][3] == expectedC[0][3]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[1][3] == expectedC[1][3]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
    REQUIRE(matC[2][3] == expectedC[2][3]);
    REQUIRE(matC[3][0] == expectedC[3][0]);
    REQUIRE(matC[3][1] == expectedC[3][1]);
    REQUIRE(matC[3][2] == expectedC[3][2]);
    REQUIRE(matC[3][3] == expectedC[3][3]);

    Matrix4 matD = 1.0f - matA;
    glm::mat4 expectedD = 1.0f - expectedA;
    REQUIRE(matD[0][0] == expectedD[0][0]);
    REQUIRE(matD[0][1] == expectedD[0][1]);
    REQUIRE(matD[0][2] == expectedD[0][2]);
    REQUIRE(matD[0][3] == expectedD[0][3]);
    REQUIRE(matD[1][0] == expectedD[1][0]);
    REQUIRE(matD[1][1] == expectedD[1][1]);
    REQUIRE(matD[1][2] == expectedD[1][2]);
    REQUIRE(matD[1][3] == expectedD[1][3]);
    REQUIRE(matD[2][0] == expectedD[2][0]);
    REQUIRE(matD[2][1] == expectedD[2][1]);
    REQUIRE(matD[2][2] == expectedD[2][2]);
    REQUIRE(matD[2][3] == expectedD[2][3]);
    REQUIRE(matD[3][0] == expectedD[3][0]);
    REQUIRE(matD[3][1] == expectedD[3][1]);
    REQUIRE(matD[3][2] == expectedD[3][2]);
    REQUIRE(matD[3][3] == expectedD[3][3]);
  }

  SECTION("Matrix Multiplication")
  {
    Matrix4 matC = matA * matB;
    glm::mat4 expectedC = expectedA * expectedB;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[0][3] == expectedC[0][3]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[1][3] == expectedC[1][3]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
    REQUIRE(matC[2][3] == expectedC[2][3]);
    REQUIRE(matC[3][0] == expectedC[3][0]);
    REQUIRE(matC[3][1] == expectedC[3][1]);
    REQUIRE(matC[3][2] == expectedC[3][2]);
    REQUIRE(matC[3][3] == expectedC[3][3]);
  }

  SECTION("Scalar Multiplication")
  {
    Matrix4 matC = matA * 2.0f;
    glm::mat4 expectedC = expectedA * 2.0f;
    REQUIRE(matC[0][0] == expectedC[0][0]);
    REQUIRE(matC[0][1] == expectedC[0][1]);
    REQUIRE(matC[0][2] == expectedC[0][2]);
    REQUIRE(matC[0][3] == expectedC[0][3]);
    REQUIRE(matC[1][0] == expectedC[1][0]);
    REQUIRE(matC[1][1] == expectedC[1][1]);
    REQUIRE(matC[1][2] == expectedC[1][2]);
    REQUIRE(matC[1][3] == expectedC[1][3]);
    REQUIRE(matC[2][0] == expectedC[2][0]);
    REQUIRE(matC[2][1] == expectedC[2][1]);
    REQUIRE(matC[2][2] == expectedC[2][2]);
    REQUIRE(matC[2][3] == expectedC[2][3]);
    REQUIRE(matC[3][0] == expectedC[3][0]);
    REQUIRE(matC[3][1] == expectedC[3][1]);
    REQUIRE(matC[3][2] == expectedC[3][2]);
    REQUIRE(matC[3][3] == expectedC[3][3]);

    Matrix4 matD = 2.0f * matA;
    glm::mat4 expectedD = 2.0f * expectedA;
    REQUIRE(matD[0][0] == expectedD[0][0]);
    REQUIRE(matD[0][1] == expectedD[0][1]);
    REQUIRE(matD[0][2] == expectedD[0][2]);
    REQUIRE(matD[0][3] == expectedD[0][3]);
    REQUIRE(matD[1][0] == expectedD[1][0]);
    REQUIRE(matD[1][1] == expectedD[1][1]);
    REQUIRE(matD[1][2] == expectedD[1][2]);
    REQUIRE(matD[1][3] == expectedD[1][3]);
    REQUIRE(matD[2][0] == expectedD[2][0]);
    REQUIRE(matD[2][1] == expectedD[2][1]);
    REQUIRE(matD[2][2] == expectedD[2][2]);
    REQUIRE(matD[2][3] == expectedD[2][3]);
    REQUIRE(matD[3][0] == expectedD[3][0]);
    REQUIRE(matD[3][1] == expectedD[3][1]);
    REQUIRE(matD[3][2] == expectedD[3][2]);
    REQUIRE(matD[3][3] == expectedD[3][3]);
  }

  SECTION("Vector4 Mulitplication")
  {
    Vector4 vec = matA * Vector4(4.0f, -3.5f, 8.0f, -2.2f);
    glm::vec4 expected = expectedA * glm::vec4(4.0f, -3.5f, 8.0f, -2.2f);
    REQUIRE(vec[0] == Approx(expected.x));
    REQUIRE(vec[1] == Approx(expected.y));
    REQUIRE(vec[2] == Approx(expected.z));
    REQUIRE(vec[3] == Approx(expected.w));
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
  Quaternion orientation(Degree(23), Degree(54), Degree(-14));

  SECTION("Rotation")
  {
    Matrix4 rotResult(Matrix4::Rotation(orientation));
    glm::mat4 expected(glm::toMat4(glm::normalize(glm::quat(glm::vec3(glm::radians(23.0f), glm::radians(54.0f), glm::radians(-14.0f))))));

    REQUIRE(rotResult[0][0] == Approx(expected[0][0]));
    REQUIRE(rotResult[0][1] == Approx(expected[0][1]));
    REQUIRE(rotResult[0][2] == Approx(expected[0][2]));
    REQUIRE(rotResult[0][3] == Approx(expected[0][3]));
    REQUIRE(rotResult[1][0] == Approx(expected[1][0]));
    REQUIRE(rotResult[1][1] == Approx(expected[1][1]));
    REQUIRE(rotResult[1][2] == Approx(expected[1][2]));
    REQUIRE(rotResult[1][3] == Approx(expected[1][3]));
    REQUIRE(rotResult[2][0] == Approx(expected[2][0]));
    REQUIRE(rotResult[2][1] == Approx(expected[2][1]));
    REQUIRE(rotResult[2][2] == Approx(expected[2][2]));
    REQUIRE(rotResult[2][3] == Approx(expected[2][3]));
    REQUIRE(rotResult[3][0] == Approx(expected[3][0]));
    REQUIRE(rotResult[3][1] == Approx(expected[3][1]));
    REQUIRE(rotResult[3][2] == Approx(expected[3][2]));
    REQUIRE(rotResult[3][3] == Approx(expected[3][3]));
  }

  SECTION("Translation")
  {
    Matrix4 result(Matrix4::Translation(position));
    glm::mat4 expected = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 3.0f, 1.0f));
    REQUIRE(result[0][0] == Approx(expected[0][0]));
    REQUIRE(result[0][1] == Approx(expected[0][1]));
    REQUIRE(result[0][2] == Approx(expected[0][2]));
    REQUIRE(result[0][3] == Approx(expected[0][3]));
    REQUIRE(result[1][0] == Approx(expected[1][0]));
    REQUIRE(result[1][1] == Approx(expected[1][1]));
    REQUIRE(result[1][2] == Approx(expected[1][2]));
    REQUIRE(result[1][3] == Approx(expected[1][3]));
    REQUIRE(result[2][0] == Approx(expected[2][0]));
    REQUIRE(result[2][1] == Approx(expected[2][1]));
    REQUIRE(result[2][2] == Approx(expected[2][2]));
    REQUIRE(result[2][3] == Approx(expected[2][3]));
    REQUIRE(result[3][0] == Approx(expected[3][0]));
    REQUIRE(result[3][1] == Approx(expected[3][1]));
    REQUIRE(result[3][2] == Approx(expected[3][2]));
    REQUIRE(result[3][3] == Approx(expected[3][3]));
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
  Matrix4 matA(4.3f, 7.9f, 2.4f, -2.5f,
               6.1f, -3.9f, 7.8f, 1.1f,
               3.1f, 5.5f, -9.2f, 0.4f,
               0.5f, 4.2f, 9.4f, 8.3f);
  REQUIRE(matA.Determinate() == Approx(8253.817000f));
}

TEST_CASE("Matrix4 Inverse")
{
  Matrix4 matA(4.3f, 7.9f, 2.4f, -2.5f,
               6.1f, -3.9f, 7.8f, 1.1f,
               3.1f, 5.5f, -9.2f, 0.4f,
               0.5f, 4.2f, 9.4f, 8.3f);
  glm::mat4 expected(glm::inverse(glm::mat4(4.3f, 7.9f, 2.4f, -2.5f,
                                            6.1f, -3.9f, 7.8f, 1.1f,
                                            3.1f, 5.5f, -9.2f, 0.4f,
                                            0.5f, 4.2f, 9.4f, 8.3f)));
  Matrix4 result = matA.Inverse();
  REQUIRE(result[0][0] == Approx(expected[0][0]));
  REQUIRE(result[0][1] == Approx(expected[0][1]));
  REQUIRE(result[0][2] == Approx(expected[0][2]));
  REQUIRE(result[0][3] == Approx(expected[0][3]));
  REQUIRE(result[1][0] == Approx(expected[1][0]));
  REQUIRE(result[1][1] == Approx(expected[1][1]));
  REQUIRE(result[1][2] == Approx(expected[1][2]));
  REQUIRE(result[1][3] == Approx(expected[1][3]));
  REQUIRE(result[2][0] == Approx(expected[2][0]));
  REQUIRE(result[2][1] == Approx(expected[2][1]));
  REQUIRE(result[2][2] == Approx(expected[2][2]));
  REQUIRE(result[2][3] == Approx(expected[2][3]));
  REQUIRE(result[3][0] == Approx(expected[3][0]));
  REQUIRE(result[3][1] == Approx(expected[3][1]));
  REQUIRE(result[3][2] == Approx(expected[3][2]));
  REQUIRE(result[3][3] == Approx(expected[3][3]));
}

TEST_CASE("Matrix4 Transpose")
{
  Matrix4 matC(4.3f, 7.9f, 2.4f, -2.5f,
               6.1f, -3.9f, 7.8f, 1.1f,
               3.1f, 5.5f, -9.2f, 0.4f,
               0.5f, 4.2f, 9.4f, 8.3f);
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

TEST_CASE("MATRIX PERSPECTIVE")
{
  Matrix4 result(Matrix4::Perspective(Degree(70.0f), 1.5f, 0.1f, 1000.0f));
  glm::mat4 expected(glm::perspective(glm::radians(70.0f), 1.5f, 0.1f, 1000.0f));

  REQUIRE(result[0][0] == Approx(expected[0][0]));
  REQUIRE(result[0][1] == Approx(expected[0][1]));
  REQUIRE(result[0][2] == Approx(expected[0][2]));
  REQUIRE(result[0][3] == Approx(expected[0][3]));
  REQUIRE(result[1][0] == Approx(expected[1][0]));
  REQUIRE(result[1][1] == Approx(expected[1][1]));
  REQUIRE(result[1][2] == Approx(expected[1][2]));
  REQUIRE(result[1][3] == Approx(expected[1][3]));
  REQUIRE(result[2][0] == Approx(expected[2][0]));
  REQUIRE(result[2][1] == Approx(expected[2][1]));
  REQUIRE(result[2][2] == Approx(expected[2][2]));
  REQUIRE(result[2][3] == Approx(expected[2][3]));
  REQUIRE(result[3][0] == Approx(expected[3][0]));
  REQUIRE(result[3][1] == Approx(expected[3][1]));
  REQUIRE(result[3][2] == Approx(expected[3][2]));
  REQUIRE(result[3][3] == Approx(expected[3][3]));
}
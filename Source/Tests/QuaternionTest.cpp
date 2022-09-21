#include "catch.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../Engine/Maths/Degree.hpp"
#include "../Engine/Maths/Math.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Matrix4.hpp"
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

  Degree angle(23);
  Vector3 axis(4, 3, 2);
  axis.Normalize();
  Quaternion qatD(axis, angle);
  glm::quat resultAngleAxis = glm::angleAxis(angle.InRadians(), glm::vec3(axis[0], axis[1], axis[2]));
  REQUIRE(qatD[0] == Approx(resultAngleAxis[0]));
  REQUIRE(qatD[1] == Approx(resultAngleAxis[1]));
  REQUIRE(qatD[2] == Approx(resultAngleAxis[2]));
  REQUIRE(qatD[3] == Approx(resultAngleAxis[3]));

  Quaternion qatE(angle, angle, angle);
  glm::quat result(glm::vec3(angle.InRadians(), angle.InRadians(), angle.InRadians()));
  REQUIRE(qatE[0] == Approx(result[0]));
  REQUIRE(qatE[1] == Approx(result[1]));
  REQUIRE(qatE[2] == Approx(result[2]));
  REQUIRE(qatE[3] == Approx(result[3]));

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

  SECTION("FROM ROTATION MATRIX")
  {
    glm::vec3 axisOfRotation(glm::normalize(glm::vec3(2, 3, 4)));
    glm::mat3 expectedRotation(glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), axisOfRotation)));
    Matrix3 rotation(expectedRotation[0][0], expectedRotation[0][1], expectedRotation[0][2],
                     expectedRotation[1][0], expectedRotation[1][1], expectedRotation[1][2],
                     expectedRotation[2][0], expectedRotation[2][1], expectedRotation[2][2]);
    Quaternion result(rotation);
    glm::quat expected(expectedRotation);

    REQUIRE(result[0] == Approx(expected[0]));
    REQUIRE(result[1] == Approx(expected[1]));
    REQUIRE(result[2] == Approx(expected[2]));
    REQUIRE(result[3] == Approx(expected[3]));
  }
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
    Quaternion multiplicand(2, 3, 4, 5);
    Quaternion multiplier(5, 6, 2, 1);
    Quaternion qatC = multiplicand * multiplier;
    glm::quat expected(glm::quat(multiplicand.W, multiplicand.X, multiplicand.Y, multiplicand.Z) *
                       glm::quat(multiplier.W, multiplier.X, multiplier.Y, multiplier.Z));
    REQUIRE(qatC[0] == Approx(expected[0]));
    REQUIRE(qatC[1] == Approx(expected[1]));
    REQUIRE(qatC[2] == Approx(expected[2]));
    REQUIRE(qatC[3] == Approx(expected[3]));

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

TEST_CASE("Quaternion Euler Conversion")
{
  Quaternion quat(4, 3, 6, 7);
  std::array<Radian, 3> euler(quat.ToEuler());

  glm::vec3 result(glm::eulerAngles(glm::quat(4, 3, 6, 7)));
  REQUIRE(euler[0].InRadians() == result[0]);
  REQUIRE(euler[1].InRadians() == result[1]);
  REQUIRE(euler[2].InRadians() == result[2]);
}

TEST_CASE("Quaternion Look-at")
{
  Vector3 direction(4, 5, 6);

  glm::quat expected(glm::quatLookAt(glm::vec3(4, 5, 6), glm::vec3(0, 1, 0)));
  Quaternion result(Quaternion::LookAt(direction, Vector3(0, 1, 0)));

  REQUIRE(result[0] == Approx(expected[0]).margin(0.001f));
  REQUIRE(result[1] == Approx(expected[1]).margin(0.001f));
  REQUIRE(result[2] == Approx(expected[2]).margin(0.001f));
  REQUIRE(result[3] == Approx(expected[3]).margin(0.001f));
}

TEST_CASE("Vector3 Rotation")
{
  Vector3 vectorToRotate(3, 4, 5);
  Quaternion quat(5, 6, 7, 8);
  glm::vec3 expected(glm::rotate(glm::quat(quat.W, quat.X, quat.Y, quat.Z), glm::vec3(vectorToRotate[0], vectorToRotate[1], vectorToRotate[2])));
  Vector3 result(quat.Rotate(vectorToRotate));

  REQUIRE(result[0] == Approx(expected[0]));
  REQUIRE(result[1] == Approx(expected[1]));
  REQUIRE(result[2] == Approx(expected[2]));
}
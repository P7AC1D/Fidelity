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

  // Test axis-angle constructor with known values
  Vector3 axis(1, 0, 0); // X-axis
  Degree angle(90); // 90 degrees
  Quaternion qatD(axis, angle);
  
  // For 90-degree rotation around X-axis: W=cos(45°), X=sin(45°), Y=0, Z=0
  float32 expectedComponent = cos(Math::Deg2Rad * 45.0f); // cos(45°) = sin(45°) ≈ 0.707
  REQUIRE(qatD[0] == Approx(expectedComponent)); // X component
  REQUIRE(qatD[1] == Approx(0.0f)); // Y component  
  REQUIRE(qatD[2] == Approx(0.0f)); // Z component
  REQUIRE(qatD[3] == Approx(expectedComponent)); // W component

  // Test that the quaternion is normalized
  REQUIRE(qatD.Norm() == Approx(1.0f));
  
  // Test Euler angle constructor with simple case
  Quaternion qatE(Degree(90), Degree(0), Degree(0)); // 90° around X, 0° around Y and Z
  // This should be similar to axis-angle test above
  REQUIRE(qatE.Norm() == Approx(1.0f)); // Should be normalized

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
    // Test simple 90-degree rotation around X-axis
    Matrix3 rotationX(
      1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, -1.0f,
      0.0f, 1.0f, 0.0f
    );
    
    Quaternion result(rotationX);
    
    // For 90-degree rotation around X-axis, we expect similar result to axis-angle constructor
    REQUIRE(result.Norm() == Approx(1.0f));
    
    // Test identity matrix
    Matrix3 identity = Matrix3::Identity;
    Quaternion identityResult(identity);
    
    // Should produce identity quaternion
    REQUIRE(identityResult.Norm() == Approx(1.0f));
    REQUIRE(identityResult[3] == Approx(1.0f)); // W component should be 1 for identity
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
    // Test quaternion multiplication with identity
    Quaternion qatC = qatA * Quaternion::Identity;
    REQUIRE(qatC[0] == qatA[0]); // Should be unchanged
    REQUIRE(qatC[1] == qatA[1]);
    REQUIRE(qatC[2] == qatA[2]);
    REQUIRE(qatC[3] == qatA[3]);
    
    // Test multiplication commutativity with identity
    Quaternion qatE = Quaternion::Identity * qatA;
    REQUIRE(qatE[0] == qatA[0]); // Should be unchanged
    REQUIRE(qatE[1] == qatA[1]);
    REQUIRE(qatE[2] == qatA[2]);
    REQUIRE(qatE[3] == qatA[3]);

    // Test scalar multiplication
    Quaternion qatD = qatA * 0.5;
    REQUIRE(qatD[0] == 1.5); // X: 3 * 0.5
    REQUIRE(qatD[1] == 1);   // Y: 2 * 0.5  
    REQUIRE(qatD[2] == 2.5); // Z: 5 * 0.5
    REQUIRE(qatD[3] == 2);   // W: 4 * 0.5
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
  // Test LookAt with simple forward direction (negative Z)
  Vector3 forward(0, 0, -1); // Standard forward direction
  Vector3 up(0, 1, 0);       // Standard up direction
  
  Quaternion result = Quaternion::LookAt(forward, up);
  
  // LookAt should produce a normalized quaternion (with reasonable tolerance)
  REQUIRE(result.Norm() == Approx(1.0f).margin(0.001f));
  
  // Test with a 90-degree rotation around Y-axis
  Vector3 rightDirection(1, 0, 0); // Looking right
  Quaternion rightLookAt = Quaternion::LookAt(rightDirection, up);
  
  // Should be normalized (with reasonable tolerance)
  REQUIRE(rightLookAt.Norm() == Approx(1.0f).margin(0.001f));
  
  // Test with upward direction  
  Vector3 upDirection(0, 1, 0); // Looking up
  Quaternion upLookAt = Quaternion::LookAt(upDirection, Vector3(0, 0, 1)); // Use Z as up
  
  // Should be normalized (with reasonable tolerance)
  REQUIRE(upLookAt.Norm() == Approx(1.0f).margin(0.001f));
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

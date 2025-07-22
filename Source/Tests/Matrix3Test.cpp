#include "catch.hpp"

#include <glm/gtx/quaternion.hpp>

#include "../Engine/Maths/Matrix3.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Vector3.hpp"

// Helper macro to compare row-major Matrix3 with column-major glm::mat3
#define REQUIRE_MATRIX3_EQUALS(ourMat, glmMat) \
  REQUIRE(ourMat[0][0] == glmMat[0][0]); \
  REQUIRE(ourMat[0][1] == glmMat[1][0]); \
  REQUIRE(ourMat[0][2] == glmMat[2][0]); \
  REQUIRE(ourMat[1][0] == glmMat[0][1]); \
  REQUIRE(ourMat[1][1] == glmMat[1][1]); \
  REQUIRE(ourMat[1][2] == glmMat[2][1]); \
  REQUIRE(ourMat[2][0] == glmMat[0][2]); \
  REQUIRE(ourMat[2][1] == glmMat[1][2]); \
  REQUIRE(ourMat[2][2] == glmMat[2][2]);

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

  // Create GLM matrices to match our row-major layout
  // GLM constructor takes arguments in column-major order
  glm::mat3 expectedA;
  expectedA[0] = glm::vec3(3.9f, 7.7f, 8.2f);  // first column = our first row
  expectedA[1] = glm::vec3(2.1f, 8.3f, 4.4f);  // second column = our second row  
  expectedA[2] = glm::vec3(9.0f, 5.7f, 1.3f);  // third column = our third row
  
  glm::mat3 expectedB;
  expectedB[0] = glm::vec3(2.1f, 7.0f, 4.7f);  // first column = our first row
  expectedB[1] = glm::vec3(5.5f, 2.2f, 3.0f);  // second column = our second row
  expectedB[2] = glm::vec3(9.2f, 1.5f, 2.1f);  // third column = our third row

  SECTION("Addition")
  {
    Matrix3 matC = matA + matB;
    glm::mat3 expectedC = expectedA + expectedB;
    REQUIRE_MATRIX3_EQUALS(matC, expectedC);

    Matrix3 matD = matA + 2.4f;
    glm::mat3 expectedD = expectedA + 2.4f;
    REQUIRE_MATRIX3_EQUALS(matD, expectedD);

    Matrix3 matE = 2.4f + matA;
    glm::mat3 expectedE = 2.4f + expectedA;
    REQUIRE_MATRIX3_EQUALS(matE, expectedE);
  }

  SECTION("Subtraction")
  {
    Matrix3 matC = matA - matB;
    glm::mat3 expectedC = expectedA + -expectedB;
    REQUIRE_MATRIX3_EQUALS(matC, expectedC);

    Matrix3 matD = matA - 2.4f;
    glm::mat3 expectedD = expectedA - 2.4f;
    REQUIRE_MATRIX3_EQUALS(matD, expectedD);

    Matrix3 matE = 2.4f - matA;
    glm::mat3 expectedE = 2.4f - expectedA;
    REQUIRE_MATRIX3_EQUALS(matE, expectedE);
  }

  SECTION("Multiplication")
  {
    Matrix3 matC = matA * matB;
    glm::mat3 expectedC = expectedA * expectedB;
    REQUIRE_MATRIX3_EQUALS(matC, expectedC);

    Matrix3 matD = matA * 2.4f;
    glm::mat3 expectedD = expectedA * 2.4f;
    REQUIRE_MATRIX3_EQUALS(matD, expectedD);

    Matrix3 matE = 2.4f * matA;
    glm::mat3 expectedE = 2.4f * expectedA;
    REQUIRE_MATRIX3_EQUALS(matE, expectedE);
  }

  SECTION("Addition Assignment")
  {
    Matrix3 matC(matA);
    glm::mat3 expectedC(expectedA);
    matC += 2.0f;
    expectedC += 2.0f;
    REQUIRE_MATRIX3_EQUALS(matC, expectedC);
  }

  SECTION("Subtraction Assignment")
  {
    Matrix3 matC(matA);
    glm::mat3 expectedC(expectedA);
    matC -= 2.0f;
    expectedC -= 2.0f;
    REQUIRE_MATRIX3_EQUALS(matC, expectedC);
  }

  SECTION("Mulitplication Assignment")
  {
    Matrix3 matC(matA);
    glm::mat3 expectedC(expectedA);
    matC *= 2.0f;
    expectedC *= 2.0f;
    REQUIRE_MATRIX3_EQUALS(matC, expectedC);
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
    REQUIRE_MATRIX3_EQUALS(matC, expectedC);
  }

  SECTION("Transpose")
  {
    Matrix3 matC = matA.Transpose();
    glm::mat3 expectedC(glm::transpose(expectedA));
    REQUIRE_MATRIX3_EQUALS(matC, expectedC);
  }

  SECTION("Look-At")
  {
    Vector3 from(0.0f, 0.0f, 0.0f);
    Vector3 to(1.0f, 1.0f, 1.0f);
    glm::mat3 expected = glm::lookAt(glm::vec3(from.X, from.Y, from.Z), glm::vec3(to.X, to.Y, to.Z), glm::vec3(0.0f, 1.0f, 0.0f));
    Matrix3 result = Matrix3::LookAt(from, to, Vector3::Up);

    // Use transpose pattern for row-major vs column-major comparison
    REQUIRE(result[0][0] == Approx(expected[0][0]).margin(0.001f));
    REQUIRE(result[0][1] == Approx(expected[1][0]).margin(0.001f));
    REQUIRE(result[0][2] == Approx(expected[2][0]).margin(0.001f));
    REQUIRE(result[1][0] == Approx(expected[0][1]).margin(0.001f));
    REQUIRE(result[1][1] == Approx(expected[1][1]).margin(0.001f));
    REQUIRE(result[1][2] == Approx(expected[2][1]).margin(0.001f));
    REQUIRE(result[2][0] == Approx(expected[0][2]).margin(0.001f));
    REQUIRE(result[2][1] == Approx(expected[1][2]).margin(0.001f));
    REQUIRE(result[2][2] == Approx(expected[2][2]).margin(0.001f));
  }
}

TEST_CASE("Matrix3 Multiplication Correctness Verification")
{
  SECTION("Identity Matrix Multiplication")
  {
    Matrix3 matA(2.0f, 3.0f, 4.0f,
                 5.0f, 6.0f, 7.0f,
                 8.0f, 9.0f, 10.0f);
    Matrix3 identity = Matrix3::Identity;
    
    Matrix3 resultLeft = matA * identity;
    Matrix3 resultRight = identity * matA;
    
    // Matrix * Identity should equal the original matrix
    REQUIRE(resultLeft == matA);
    REQUIRE(resultRight == matA);
  }
  
  SECTION("Known Matrix Multiplication")
  {
    // Test with simple matrices for manual verification
    Matrix3 matA(1.0f, 2.0f, 3.0f,
                 4.0f, 5.0f, 6.0f,
                 7.0f, 8.0f, 9.0f);
    Matrix3 matB(1.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 2.0f);
    
    Matrix3 result = matA * matB;
    
    // Expected result: [1,2,6; 4,5,12; 7,8,18]
    REQUIRE(result[0][0] == 1.0f);
    REQUIRE(result[0][1] == 2.0f);
    REQUIRE(result[0][2] == 6.0f);
    REQUIRE(result[1][0] == 4.0f);
    REQUIRE(result[1][1] == 5.0f);
    REQUIRE(result[1][2] == 12.0f);
    REQUIRE(result[2][0] == 7.0f);
    REQUIRE(result[2][1] == 8.0f);
    REQUIRE(result[2][2] == 18.0f);
  }
  
  SECTION("Matrix Multiplication Associativity")
  {
    Matrix3 matA(1.0f, 2.0f, 0.0f,
                 0.0f, 1.0f, 1.0f,
                 1.0f, 0.0f, 1.0f);
    Matrix3 matB(2.0f, 0.0f, 1.0f,
                 1.0f, 1.0f, 0.0f,
                 0.0f, 1.0f, 1.0f);
    Matrix3 matC(1.0f, 1.0f, 1.0f,
                 0.0f, 1.0f, 0.0f,
                 1.0f, 0.0f, 1.0f);
    
    Matrix3 resultABC = (matA * matB) * matC;
    Matrix3 resultACB = matA * (matB * matC);
    
    // Matrix multiplication should be associative: (A*B)*C = A*(B*C)
    const float32 epsilon = 0.0001f;
    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        REQUIRE(resultABC[i][j] == Approx(resultACB[i][j]).epsilon(epsilon));
      }
    }
  }
}

TEST_CASE("Matrix3 Multiplication Correctness", "[Matrix3]")
{
  // Test specific case to verify matrix multiplication is correct
  Matrix3 matA(1, 2, 3,
               4, 5, 6,
               7, 8, 9);
  
  Matrix3 matB(2, 0, 1,
               0, 1, 0,
               1, 0, 2);
  
  // Expected result calculated manually: 
  // Row 0: [1*2+2*0+3*1, 1*0+2*1+3*0, 1*1+2*0+3*2] = [5, 2, 7]
  // Row 1: [4*2+5*0+6*1, 4*0+5*1+6*0, 4*1+5*0+6*2] = [14, 5, 16]  
  // Row 2: [7*2+8*0+9*1, 7*0+8*1+9*0, 7*1+8*0+9*2] = [23, 8, 25]
  Matrix3 expected(5, 2, 7,
                   14, 5, 16,
                   23, 8, 25);
  
  Matrix3 result = matA * matB;
  
  REQUIRE(result[0][0] == expected[0][0]);
  REQUIRE(result[0][1] == expected[0][1]);
  REQUIRE(result[0][2] == expected[0][2]);
  REQUIRE(result[1][0] == expected[1][0]);
  REQUIRE(result[1][1] == expected[1][1]);
  REQUIRE(result[1][2] == expected[1][2]);
  REQUIRE(result[2][0] == expected[2][0]);
  REQUIRE(result[2][1] == expected[2][1]);
  REQUIRE(result[2][2] == expected[2][2]);
  
  // Test identity matrix multiplication
  Matrix3 identity = Matrix3::Identity;
  Matrix3 identityResult = matA * identity;
  
  REQUIRE(identityResult[0][0] == matA[0][0]);
  REQUIRE(identityResult[0][1] == matA[0][1]);
  REQUIRE(identityResult[0][2] == matA[0][2]);
  REQUIRE(identityResult[1][0] == matA[1][0]);
  REQUIRE(identityResult[1][1] == matA[1][1]);
  REQUIRE(identityResult[1][2] == matA[1][2]);
  REQUIRE(identityResult[2][0] == matA[2][0]);
  REQUIRE(identityResult[2][1] == matA[2][1]);
  REQUIRE(identityResult[2][2] == matA[2][2]);
}

TEST_CASE("Matrix3 Storage Order Investigation")
{
  SECTION("Construction and Access Pattern")
  {
    // Create a simple test matrix
    Matrix3 ourMat(1.0f, 2.0f, 3.0f,
                   4.0f, 5.0f, 6.0f,
                   7.0f, 8.0f, 9.0f);
    
    // Check if our matrix matches expected row-major layout
    REQUIRE(ourMat[0][0] == 1.0f);
    REQUIRE(ourMat[0][1] == 2.0f);
    REQUIRE(ourMat[0][2] == 3.0f);
    REQUIRE(ourMat[1][0] == 4.0f);
    REQUIRE(ourMat[1][1] == 5.0f);
    REQUIRE(ourMat[1][2] == 6.0f);
    REQUIRE(ourMat[2][0] == 7.0f);
    REQUIRE(ourMat[2][1] == 8.0f);
    REQUIRE(ourMat[2][2] == 9.0f);
  }
  
  SECTION("GLM vs Our Matrix Layout")
  {
    // GLM mat3 constructor takes arguments in column-major order
    // but we need to access them correctly for comparison
    Matrix3 ourMat(1.0f, 2.0f, 3.0f,
                   4.0f, 5.0f, 6.0f,
                   7.0f, 8.0f, 9.0f);
                   
    // GLM constructor format: mat3(col0, col1, col2) where each col is a vec3
    // But when we construct with scalars, it goes:
    // mat3(m00, m10, m20, m01, m11, m21, m02, m12, m22)
    // which is column-major layout
    glm::mat3 glmMat(1.0f, 4.0f, 7.0f,  // first column
                     2.0f, 5.0f, 8.0f,  // second column  
                     3.0f, 6.0f, 9.0f); // third column
    
    // Now they should match when accessed as [row][col]
    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        REQUIRE(ourMat[i][j] == Approx(glmMat[j][i]).epsilon(0.0001f));
      }
    }
  }
}

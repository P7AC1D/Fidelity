#include "catch.hpp"

#include <cmath>

#include "../Engine/Maths/Vector2.hpp"
#include "../Engine/Maths/Vector3.hpp"

TEST_CASE("Vector3 operations: ", "[Vector3]")
{
  float32 k = 3.0f;
  Vector3 vec3A(4);
  Vector3 vec3B(7, 3, 8);
  
  SECTION("Constructor and Accessor")
  {
    Vector3 vec3C;
    REQUIRE(vec3C[0] == 0);
    REQUIRE(vec3C[1] == 0);
    REQUIRE(vec3C[2] == 0);
    
    REQUIRE(vec3A[0] == 4);
    REQUIRE(vec3A[1] == 4);
    REQUIRE(vec3A[2] == 4);
    
    REQUIRE(vec3B[0] == 7);
    REQUIRE(vec3B[1] == 3);
    REQUIRE(vec3B[2] == 8);
    
    Vector3 vec3D(Vector2(3, 2));
    REQUIRE(vec3D[0] == 3);
    REQUIRE(vec3D[1] == 2);
    REQUIRE(vec3D[2] == 0);
    
    Vector3 vec3E(Vector2(3, 2), 9);
    REQUIRE(vec3E[0] == 3);
    REQUIRE(vec3E[1] == 2);
    REQUIRE(vec3E[2] == 9);
    
    Vector3 vec3F(vec3B);
    REQUIRE(vec3F[0] == 7);
    REQUIRE(vec3F[1] == 3);
    REQUIRE(vec3F[2] == 8);
  }
  
  SECTION("Copy-Assignment")
  {
    Vector3 vec3C = vec3B;
    REQUIRE(vec3C[0] == 7);
    REQUIRE(vec3C[1] == 3);
    REQUIRE(vec3C[2] == 8);
  }
  
  SECTION("Addition Binary Operators")
  {
    vec3B += vec3A;
    REQUIRE(vec3B[0] == 11);
    REQUIRE(vec3B[1] == 7);
    REQUIRE(vec3B[2] == 12);
    
    vec3B += 4;
    REQUIRE(vec3B[0] == 15);
    REQUIRE(vec3B[1] == 11);
    REQUIRE(vec3B[2] == 16);
    
    Vector3 vec3C = 7 + vec3B;
    REQUIRE(vec3C[0] == 22);
    REQUIRE(vec3C[1] == 18);
    REQUIRE(vec3C[2] == 23);
    
    Vector3 vec3D = vec3C + vec3B;
    REQUIRE(vec3D[0] == 37);
    REQUIRE(vec3D[1] == 29);
    REQUIRE(vec3D[2] == 39);
  }
  
  SECTION("Subtraction Binary Operators")
  {
    vec3B -= Vector3(4, 2, 6);
    REQUIRE(vec3B[0] == 3);
    REQUIRE(vec3B[1] == 1);
    REQUIRE(vec3B[2] == 2);
    
    vec3B -= 2;
    REQUIRE(vec3B[0] == 1);
    REQUIRE(vec3B[1] == -1);
    REQUIRE(vec3B[2] == 0);
    
    Vector3 vec3C = 5 - vec3B;
    REQUIRE(vec3C[0] == 4);
    REQUIRE(vec3C[1] == 6);
    REQUIRE(vec3C[2] == 5);
    
    Vector3 vec3D = vec3B - vec3A;
    REQUIRE(vec3D[0] == -3);
    REQUIRE(vec3D[1] == -5);
    REQUIRE(vec3D[2] == -4);
  }
  
  SECTION("Mulitplication Binary Operators")
  {
    vec3B *= 3;
    REQUIRE(vec3B[0] == 21);
    REQUIRE(vec3B[1] == 9);
    REQUIRE(vec3B[2] == 24);
    
    Vector3 vec3C = vec3B * 4;
    REQUIRE(vec3C[0] == 84);
    REQUIRE(vec3C[1] == 36);
    REQUIRE(vec3C[2] == 96);
    
    Vector3 vec3D = 4 * vec3B;
    REQUIRE(vec3D[0] == 84);
    REQUIRE(vec3D[1] == 36);
    REQUIRE(vec3D[2] == 96);
  }
  
  SECTION("Division Binary Operators")
  {
    Vector3 vec3C = vec3B / 2.0f;
    REQUIRE(vec3C[0] == 3.5f);
    REQUIRE(vec3C[1] == 1.5f);
    REQUIRE(vec3C[2] == 4.0f);
    
    // Test component-wise division
    Vector3 vec3D = vec3B / Vector3(7.0f, 3.0f, 8.0f);
    REQUIRE(vec3D[0] == 1.0f);
    REQUIRE(vec3D[1] == 1.0f);
    REQUIRE(vec3D[2] == 1.0f);
    
    // Test division assignment
    Vector3 vec3E(vec3B);
    vec3E /= 2.0f;
    REQUIRE(vec3E[0] == 3.5f);
    REQUIRE(vec3E[1] == 1.5f);
    REQUIRE(vec3E[2] == 4.0f);
    
    // Test division by zero safety
    Vector3 vec3F = vec3B / 0.0f;
    REQUIRE(vec3F[0] == 0.0f);
    REQUIRE(vec3F[1] == 0.0f);
    REQUIRE(vec3F[2] == 0.0f);
    
    Vector3 vec3G(vec3B);
    vec3G /= 0.0f;
    REQUIRE(vec3G[0] == 0.0f);
    REQUIRE(vec3G[1] == 0.0f);
    REQUIRE(vec3G[2] == 0.0f);
  }
  
  SECTION("Relational Operators")
  {
    Vector3 vec3C(vec3B);
    REQUIRE(vec3C == vec3B);
    REQUIRE(vec3A != vec3B);
  }
  
  SECTION("Length and Normalization")
  {
    float32 length = vec3B.Length();
    REQUIRE(length == sqrtf(49 + 9 + 64));
    
    vec3B.Normalize();
    float32 lengthInv = 1.0f / length;
    REQUIRE(vec3B[0] == 7 * lengthInv);
    REQUIRE(vec3B[1] == 3 * lengthInv);
    REQUIRE(vec3B[2] == 8 * lengthInv);
    
    // Test zero vector normalization safety
    Vector3 zeroVec(0.0f, 0.0f, 0.0f);
    Vector3 originalZero = zeroVec;
    zeroVec.Normalize();
    REQUIRE(zeroVec == originalZero); // Should remain unchanged
    
    Vector3 staticNormalized = Vector3::Normalize(Vector3(0.0f, 0.0f, 0.0f));
    REQUIRE(staticNormalized == Vector3(0.0f, 0.0f, 0.0f));
  }
  
  SECTION("Vector Operators")
  {
    float32 dotProduct = Vector3::Dot(vec3B, vec3A);
    REQUIRE(dotProduct == 72);
    
    Vector3 crossProduct = Vector3::Cross(vec3B, vec3A);
    REQUIRE(crossProduct[0] == -20);
    REQUIRE(crossProduct[1] == 4);
    REQUIRE(crossProduct[2] == 16);
  }
}

TEST_CASE("Vector3 Edge Cases and Mathematical Properties")
{
  SECTION("Component-wise Operations")
  {
    Vector3 vecA(6.0f, 9.0f, 12.0f);
    Vector3 vecB(2.0f, 3.0f, 4.0f);
    
    // Test component-wise multiplication
    Vector3 mulResult = vecA * vecB;
    REQUIRE(mulResult[0] == 12.0f);  // 6 * 2
    REQUIRE(mulResult[1] == 27.0f);  // 9 * 3
    REQUIRE(mulResult[2] == 48.0f);  // 12 * 4
    
    // Test component-wise division
    Vector3 divResult = vecA / vecB;
    REQUIRE(divResult[0] == 3.0f);   // 6 / 2
    REQUIRE(divResult[1] == 3.0f);   // 9 / 3
    REQUIRE(divResult[2] == 3.0f);   // 12 / 4
  }
  
  SECTION("Cross Product Properties")
  {
    Vector3 i(1.0f, 0.0f, 0.0f);
    Vector3 j(0.0f, 1.0f, 0.0f);
    Vector3 k(0.0f, 0.0f, 1.0f);
    
    // i × j = k
    Vector3 iCrossJ = Vector3::Cross(i, j);
    REQUIRE(iCrossJ[0] == Approx(k[0]).epsilon(0.0001f));
    REQUIRE(iCrossJ[1] == Approx(k[1]).epsilon(0.0001f));
    REQUIRE(iCrossJ[2] == Approx(k[2]).epsilon(0.0001f));
    
    // j × k = i
    Vector3 jCrossK = Vector3::Cross(j, k);
    REQUIRE(jCrossK[0] == Approx(i[0]).epsilon(0.0001f));
    REQUIRE(jCrossK[1] == Approx(i[1]).epsilon(0.0001f));
    REQUIRE(jCrossK[2] == Approx(i[2]).epsilon(0.0001f));
    
    // k × i = j
    Vector3 kCrossI = Vector3::Cross(k, i);
    REQUIRE(kCrossI[0] == Approx(j[0]).epsilon(0.0001f));
    REQUIRE(kCrossI[1] == Approx(j[1]).epsilon(0.0001f));
    REQUIRE(kCrossI[2] == Approx(j[2]).epsilon(0.0001f));
    
    // Anti-commutativity: a × b = -(b × a)
    Vector3 a(2.0f, 3.0f, 1.0f);
    Vector3 b(1.0f, 4.0f, 2.0f);
    Vector3 aCrossB = Vector3::Cross(a, b);
    Vector3 bCrossA = Vector3::Cross(b, a);
    Vector3 negBCrossA = -bCrossA;
    
    REQUIRE(aCrossB[0] == Approx(negBCrossA[0]).epsilon(0.0001f));
    REQUIRE(aCrossB[1] == Approx(negBCrossA[1]).epsilon(0.0001f));
    REQUIRE(aCrossB[2] == Approx(negBCrossA[2]).epsilon(0.0001f));
  }
  
  SECTION("Dot Product Properties")
  {
    Vector3 a(3.0f, 4.0f, 0.0f);
    Vector3 b(1.0f, 0.0f, 0.0f);
    
    // Test orthogonal vectors (dot product = 0)
    Vector3 perpendicular(0.0f, 0.0f, 1.0f);
    REQUIRE(Vector3::Dot(a, perpendicular) == Approx(0.0f).epsilon(0.0001f));
    
    // Test commutativity: a·b = b·a
    float32 dotAB = Vector3::Dot(a, b);
    float32 dotBA = Vector3::Dot(b, a);
    REQUIRE(dotAB == Approx(dotBA).epsilon(0.0001f));
    
    // Test known values
    REQUIRE(Vector3::Dot(a, b) == 3.0f);  // (3*1 + 4*0 + 0*0) = 3
  }
}

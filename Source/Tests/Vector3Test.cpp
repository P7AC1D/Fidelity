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

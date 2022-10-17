#include "catch.hpp"

#include "../Engine/Core/Transform.h"
#include "../Engine/Maths/Matrix4.hpp"

TEST_CASE("TRANSFORM DIRECTIONAL VECTORS")
{
  SECTION("IDENTITY MATRIX")
  {
    Matrix4 mat(Matrix4::Identity);
    Transform transform(mat);

    Vector3 forward(transform.getForward());
    Vector3 up(transform.getUp());
    Vector3 right(transform.getRight());

    REQUIRE(forward.X == Vector3(0, 0, 1).X);
    REQUIRE(forward.Y == Vector3(0, 0, 1).Y);
    REQUIRE(forward.Z == Vector3(0, 0, 1).Z);
    REQUIRE(up.X == Vector3(0, 1, 0).X);
    REQUIRE(up.Y == Vector3(0, 1, 0).Y);
    REQUIRE(up.Z == Vector3(0, 1, 0).Z);
    REQUIRE(right.X == Vector3(1, 0, 0).X);
    REQUIRE(right.Y == Vector3(1, 0, 0).Y);
    REQUIRE(right.Z == Vector3(1, 0, 0).Z);
  }

  SECTION("LOOK-AT")
  {
    Matrix4 mat(Matrix4::LookAt(Vector3(0.0f, 0.0f, 3.0f), Vector3::Zero, Vector3::Up));
    Transform transform(mat);

    Vector3 forward(transform.getForward());
    Vector3 up(transform.getUp());
    Vector3 right(transform.getRight());

    REQUIRE(forward.X == Vector3::Normalize(Vector3(0, 0, 1)).X);
    REQUIRE(forward.Y == Vector3::Normalize(Vector3(0, 0, 1)).Y);
    REQUIRE(forward.Z == Vector3::Normalize(Vector3(0, 0, 1)).Z);
    REQUIRE(up.X == Vector3::Normalize(Vector3(0, 1, 0)).X);
    REQUIRE(up.Y == Vector3::Normalize(Vector3(0, 1, 0)).Y);
    REQUIRE(up.Z == Vector3::Normalize(Vector3(0, 1, 0)).Z);
    REQUIRE(right.X == Vector3::Normalize(Vector3(1, 0, 0)).X);
    REQUIRE(right.Y == Vector3::Normalize(Vector3(1, 0, 0)).Y);
    REQUIRE(right.Z == Vector3::Normalize(Vector3(1, 0, 0)).Z);
  }
}
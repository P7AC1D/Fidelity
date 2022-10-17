#include "catch.hpp"

#include "../Engine/Maths/Plane.hpp"

TEST_CASE("PLANE CONSTRUCTORS")
{
  SECTION("DEFAULT")
  {
    Plane plane;

    REQUIRE(plane.getNormal() == Vector3::Identity);
    REQUIRE(plane.getD() == 0.f);
  }

  SECTION("NORMAL-POINT")
  {
    Vector3 normal(1, 2, 3);
    normal.Normalize();
    Vector3 point(2, 2, 2);
    Plane plane(normal, point);

    REQUIRE(plane.getNormal().X == Approx(normal.X));
    REQUIRE(plane.getNormal().Y == Approx(normal.Y));
    REQUIRE(plane.getNormal().Z == Approx(normal.Z));
    REQUIRE(plane.getD() == Approx(-3.20714f));
  }

  SECTION("THREE-POINT")
  {
    Vector3 pointA(2, 3, 4);
    Vector3 pointB(1, 5, -1);
    Vector3 pointC(3, -3, 0);
    Plane plane(pointA, pointB, pointC);

    Vector3 expectedNormal(38, 9, -4);
    expectedNormal.Normalize();

    float32 expectedD = -Vector3::Dot(expectedNormal, pointA);

    REQUIRE(plane.getNormal().X == Approx(expectedNormal.X));
    REQUIRE(plane.getNormal().Y == Approx(expectedNormal.Y));
    REQUIRE(plane.getNormal().Z == Approx(expectedNormal.Z));
    REQUIRE(plane.getD() == Approx(expectedD));
  }
}

TEST_CASE("PLANE GET SIGNED-DISTANCE")
{
  SECTION("IN-FRONT OF PLANE")
  {
    Vector3 normal(1, 1, 1);
    normal.Normalize();
    Vector3 point(Vector3::Zero);
    Plane plane(normal, point);

    Vector3 target(Vector3(1, 1, 1));
    float32 result = plane.getSignedDistance(target);
    REQUIRE(result > 0.0f);
  }

  SECTION("BEHIND PLANE")
  {
    Vector3 normal(1, 1, 1);
    normal.Normalize();
    Vector3 point(Vector3::Zero);
    Plane plane(normal, point);

    Vector3 target(Vector3(-1, -1, -1));
    float32 result = plane.getSignedDistance(target);
    REQUIRE(result < 0.0f);
  }

  SECTION("ON PLANE")
  {
    Vector3 normal(1, 1, 1);
    normal.Normalize();
    Vector3 point(Vector3::Zero);
    Plane plane(normal, point);

    Vector3 target(point);
    float32 result = plane.getSignedDistance(target);
    REQUIRE(result == 0.0f);
  }
}
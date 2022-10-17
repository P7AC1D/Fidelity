#include "catch.hpp"

#include "../Engine/Maths/Aabb.hpp"

TEST_CASE("AABB CONSTRUCTORS")
{
  SECTION("DEFAULT")
  {
    Aabb aabb;

    REQUIRE(aabb.getCenter() == Vector3::Zero);
    REQUIRE(aabb.getExtents() == Vector3::Identity);
    REQUIRE(aabb.getRadius() == Approx(1.7320508075688772));
    REQUIRE(aabb.getMax() == Vector3(1.f, 1.f, 1.f));
    REQUIRE(aabb.getMin() == Vector3(-1.f, -1.f, -1.f));
  }

  SECTION("MAX-MIN")
  {
    Vector3 max(2.0f, 4.0f, 1.0f);
    Vector3 min(-2.0f, -2.0f, 0.0f);
    Aabb aabb(max, min);

    REQUIRE(aabb.getCenter() == Vector3(0.f, 1.f, 0.5f));
    REQUIRE(aabb.getExtents() == Vector3(2.f, 3.f, 0.5f));
    REQUIRE(aabb.getRadius() == Approx(3.640054944640259f));
    REQUIRE(aabb.getMax() == max);
    REQUIRE(aabb.getMin() == min);
  }

  SECTION("CENTER-EXTENTS")
  {
    Vector3 center(0.f, 1.f, 0.5f);
    Vector3 extents(2.f, 3.f, 0.5f);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);

    REQUIRE(aabb.getCenter() == center);
    REQUIRE(aabb.getExtents() == extents);
    REQUIRE(aabb.getRadius() == Approx(3.640054944640259f));
    REQUIRE(aabb.getMax() == Vector3(2.0f, 4.0f, 1.0f));
    REQUIRE(aabb.getMin() == Vector3(-2.0f, -2.0f, 0.0f));
  }
}

TEST_CASE("AABB-PLANE")
{
  SECTION("IN-FRONT OF PLANE")
  {
    Vector3 normal(1, 1, 1);
    normal.Normalize();
    Vector3 point(Vector3::Zero);
    Plane plane(normal, point);

    Vector3 center(2.f, 2.f, 2.f);
    Vector3 extents(1.f, 1.f, 1.0f);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);

    bool result = aabb.isOnOrForwardPlane(plane);
    REQUIRE(result == true);
  }

  SECTION("BEHIND PLANE")
  {
    Vector3 normal(1, 1, 1);
    normal.Normalize();
    Vector3 point(Vector3::Zero);
    Plane plane(normal, point);

    Vector3 center(-2.f, -2.f, -2.f);
    Vector3 extents(1.f, 1.f, 1.0f);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);

    bool result = aabb.isOnOrForwardPlane(plane);
    REQUIRE(result == false);
  }

  SECTION("PLANE INTERSECTION")
  {
    Vector3 normal(1, 1, 1);
    normal.Normalize();
    Vector3 point(Vector3::Zero);
    Plane plane(normal, point);

    Vector3 center(0.f, 0.f, 0.f);
    Vector3 extents(1.f, 1.f, 1.0f);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);

    bool result = aabb.isOnOrForwardPlane(plane);
    REQUIRE(result == true);
  }
}
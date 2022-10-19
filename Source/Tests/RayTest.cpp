#include "catch.hpp"

#include "../Engine/Maths/AABB.hpp"
#include "../Engine/Maths/Ray.hpp"

TEST_CASE("RAY-AABB INTERSECTION")
{
  SECTION("RAY INTERSECTS")
  {
    Vector3 center(5.0f, 0.0f, 0.0f);
    Vector3 extents(Vector3::Identity);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);

    Ray ray(Vector3::Zero, Vector3(1.0f, 0.0f, 0.0f));

    REQUIRE(ray.Intersects(aabb) == true);
  }

  SECTION("RAY DOES NOT INTERSECT")
  {
    Vector3 center(5.0f, 0.0f, 0.0f);
    Vector3 extents(Vector3::Identity);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);

    Ray ray(Vector3::Zero, Vector3(0.0f, 1.0f, 0.0f));

    REQUIRE(ray.Intersects(aabb) == false);
  }
}
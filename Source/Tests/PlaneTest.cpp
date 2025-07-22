#include "catch.hpp"

#include "../Engine/Maths/Plane.hpp"
#include <cmath>
#include <limits>
#include <vector>

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

TEST_CASE("PLANE NORMALIZE")
{
  SECTION("Unnormalized Plane - Normal and Point Constructor")
  {
    // Create a plane with an unnormalized normal
    Vector3 unnormalizedNormal(2.0f, 4.0f, 6.0f); // Length = sqrt(56) ≈ 7.48
    Vector3 point(1.0f, 1.0f, 1.0f);
    Plane plane(unnormalizedNormal, point);

    // The constructor should automatically normalize
    Vector3 expectedNormal = Vector3::Normalize(unnormalizedNormal);
    REQUIRE(plane.getNormal().X == Approx(expectedNormal.X));
    REQUIRE(plane.getNormal().Y == Approx(expectedNormal.Y));
    REQUIRE(plane.getNormal().Z == Approx(expectedNormal.Z));
  }

  SECTION("Explicit Normalize Call")
  {
    // Create a plane and then manually denormalize it
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 point(5.0f, 0.0f, 0.0f);
    Plane plane(normal, point);

    // Manually corrupt the normal to test normalize()
    // We'll access via reflection or create a custom test
    // For now, test that normalize doesn't crash
    plane.normalize();

    // After normalize, normal should still be unit length
    Vector3 planeNormal = plane.getNormal();
    float32 length = Vector3::Length(planeNormal);
    REQUIRE(length == Approx(1.0f));
  }
}

TEST_CASE("PLANE EDGE CASES")
{
  SECTION("Zero Normal Vector")
  {
    Vector3 zeroNormal(0.0f, 0.0f, 0.0f);
    Vector3 point(1.0f, 1.0f, 1.0f);

    // Creating a plane with zero normal should handle gracefully
    // The implementation normalizes, so this might create a degenerate plane
    Plane plane(zeroNormal, point);

    // Check that we don't crash and values are reasonable
    Vector3 normal = plane.getNormal();
    REQUIRE(std::isfinite(normal.X));
    REQUIRE(std::isfinite(normal.Y));
    REQUIRE(std::isfinite(normal.Z));
    REQUIRE(std::isfinite(plane.getD()));
  }

  SECTION("Collinear Points in Three-Point Constructor")
  {
    // Three collinear points should create a degenerate plane
    Vector3 pointA(0.0f, 0.0f, 0.0f);
    Vector3 pointB(1.0f, 1.0f, 1.0f);
    Vector3 pointC(2.0f, 2.0f, 2.0f); // Collinear with A and B

    Plane plane(pointA, pointB, pointC);

    // Cross product of parallel vectors is zero, so normal might be zero
    // Check that we handle this gracefully
    Vector3 normal = plane.getNormal();
    REQUIRE(std::isfinite(normal.X));
    REQUIRE(std::isfinite(normal.Y));
    REQUIRE(std::isfinite(normal.Z));
    REQUIRE(std::isfinite(plane.getD()));
  }

  SECTION("Very Small Normal Vector")
  {
    Vector3 tinyNormal(1e-10f, 1e-10f, 1e-10f);
    Vector3 point(1.0f, 1.0f, 1.0f);

    Plane plane(tinyNormal, point);

    // Should normalize properly even with tiny vectors
    Vector3 normal = plane.getNormal();
    float32 length = Vector3::Length(normal);
    REQUIRE(length == Approx(1.0f).margin(1e-5f));
  }

  SECTION("Very Large Values")
  {
    Vector3 largeNormal(1e6f, 1e6f, 1e6f);
    Vector3 largePoint(1e5f, 1e5f, 1e5f);

    Plane plane(largeNormal, largePoint);

    // Should handle large values without overflow
    Vector3 normal = plane.getNormal();
    float32 length = Vector3::Length(normal);
    REQUIRE(length == Approx(1.0f));
    REQUIRE(std::isfinite(plane.getD()));
  }
}

TEST_CASE("PLANE SIGNED DISTANCE PRECISION")
{
  SECTION("Very Close to Plane")
  {
    Vector3 normal(0.0f, 0.0f, 1.0f); // Z-axis normal
    Vector3 point(0.0f, 0.0f, 0.0f);  // Plane at origin
    Plane plane(normal, point);

    // Test point very close to plane
    Vector3 closePoint(0.0f, 0.0f, 1e-6f);
    float32 distance = plane.getSignedDistance(closePoint);

    REQUIRE(distance == Approx(1e-6f));
    REQUIRE(distance > 0.0f);
  }

  SECTION("Exact Distance Calculation")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f); // X-axis normal
    Vector3 point(2.0f, 0.0f, 0.0f);  // Plane at x=2
    Plane plane(normal, point);

    // Test various points
    REQUIRE(plane.getSignedDistance(Vector3(3.0f, 0.0f, 0.0f)) == Approx(1.0f));
    REQUIRE(plane.getSignedDistance(Vector3(1.0f, 0.0f, 0.0f)) == Approx(-1.0f));
    REQUIRE(plane.getSignedDistance(Vector3(2.0f, 5.0f, 10.0f)) == Approx(0.0f));
  }
}

TEST_CASE("PLANE COORDINATE SYSTEM TESTS")
{
  SECTION("XY Plane")
  {
    Vector3 normal(0.0f, 0.0f, 1.0f);
    Vector3 point(0.0f, 0.0f, 5.0f);
    Plane plane(normal, point);

    // Points above plane (positive Z) should be in front
    REQUIRE(plane.getSignedDistance(Vector3(0.0f, 0.0f, 6.0f)) > 0.0f);
    // Points below plane (negative Z) should be behind
    REQUIRE(plane.getSignedDistance(Vector3(0.0f, 0.0f, 4.0f)) < 0.0f);
    // Points on plane should have zero distance
    REQUIRE(plane.getSignedDistance(Vector3(10.0f, 10.0f, 5.0f)) == Approx(0.0f));
  }

  SECTION("XZ Plane")
  {
    Vector3 normal(0.0f, 1.0f, 0.0f);
    Vector3 point(0.0f, 3.0f, 0.0f);
    Plane plane(normal, point);

    REQUIRE(plane.getSignedDistance(Vector3(0.0f, 4.0f, 0.0f)) > 0.0f);
    REQUIRE(plane.getSignedDistance(Vector3(0.0f, 2.0f, 0.0f)) < 0.0f);
    REQUIRE(plane.getSignedDistance(Vector3(15.0f, 3.0f, 15.0f)) == Approx(0.0f));
  }

  SECTION("YZ Plane")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 point(-2.0f, 0.0f, 0.0f);
    Plane plane(normal, point);

    REQUIRE(plane.getSignedDistance(Vector3(-1.0f, 0.0f, 0.0f)) > 0.0f);
    REQUIRE(plane.getSignedDistance(Vector3(-3.0f, 0.0f, 0.0f)) < 0.0f);
    REQUIRE(plane.getSignedDistance(Vector3(-2.0f, 20.0f, 20.0f)) == Approx(0.0f));
  }
}

TEST_CASE("PLANE THREE-POINT CONSTRUCTOR VALIDATION")
{
  SECTION("Right-Hand Rule Normal Direction")
  {
    // Test that normal follows the constructor's implementation
    Vector3 pointA(1.0f, 0.0f, 0.0f);
    Vector3 pointB(0.0f, 1.0f, 0.0f);
    Vector3 pointC(0.0f, 0.0f, 1.0f);

    Plane plane(pointA, pointB, pointC);
    Vector3 normal = plane.getNormal();

    // Manual calculation matching the constructor: edgeA = pointA - pointB, edgeB = pointC - pointA
    Vector3 edgeA = pointA - pointB; // (1, -1, 0)
    Vector3 edgeB = pointC - pointA; // (-1, 0, 1)
    Vector3 expectedNormal = Vector3::Normalize(Vector3::Cross(edgeA, edgeB));

    REQUIRE(normal.X == Approx(expectedNormal.X));
    REQUIRE(normal.Y == Approx(expectedNormal.Y));
    REQUIRE(normal.Z == Approx(expectedNormal.Z));
  }

  SECTION("Different Point Orders")
  {
    Vector3 pointA(0.0f, 0.0f, 0.0f);
    Vector3 pointB(1.0f, 0.0f, 0.0f);
    Vector3 pointC(0.0f, 1.0f, 0.0f);

    Plane plane1(pointA, pointB, pointC);
    Plane plane2(pointA, pointC, pointB); // Swapped B and C

    // Normals should be opposite
    Vector3 normal1 = plane1.getNormal();
    Vector3 normal2 = plane2.getNormal();

    REQUIRE(normal1.X == Approx(-normal2.X));
    REQUIRE(normal1.Y == Approx(-normal2.Y));
    REQUIRE(normal1.Z == Approx(-normal2.Z));
  }
}

// NEW ENHANCED TESTS
TEST_CASE("PLANE ROBUSTNESS AND ERROR HANDLING")
{
  SECTION("Division by Zero Protection in normalize()")
  {
    // Create a plane with zero normal (should not crash when normalizing)
    Vector3 zeroNormal(0.0f, 0.0f, 0.0f);
    Vector3 point(1.0f, 1.0f, 1.0f);

    Plane plane(zeroNormal, point);

    // Should not crash and should create a valid default plane
    REQUIRE(plane.isValid());
    REQUIRE(std::isfinite(plane.getD()));

    // Normalizing should not crash
    plane.normalize();
    REQUIRE(plane.isValid());
  }

  SECTION("Very Small Normal Vector Handling")
  {
    Vector3 tinyNormal(1e-10f, 1e-10f, 1e-10f);
    Vector3 point(1.0f, 1.0f, 1.0f);

    Plane plane(tinyNormal, point);

    // Should create a valid normalized plane
    REQUIRE(plane.isValid());
    Vector3 normal = plane.getNormal();
    float32 length = Vector3::Length(normal);
    REQUIRE(length == Approx(1.0f).margin(1e-5f));
  }

  SECTION("Collinear Points in Three-Point Constructor")
  {
    Vector3 pointA(0.0f, 0.0f, 0.0f);
    Vector3 pointB(1.0f, 1.0f, 1.0f);
    Vector3 pointC(2.0f, 2.0f, 2.0f); // Collinear with A and B

    Plane plane(pointA, pointB, pointC);

    // Should create a valid default plane
    REQUIRE(plane.isValid());
    REQUIRE(std::isfinite(plane.getD()));
  }
}

TEST_CASE("PLANE UTILITY METHODS")
{
  SECTION("isValid() Method")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 point(5.0f, 0.0f, 0.0f);
    Plane plane(normal, point);

    REQUIRE(plane.isValid() == true);
  }

  SECTION("getPointOnPlane() Method")
  {
    Vector3 normal(0.0f, 0.0f, 1.0f);
    Vector3 inputPoint(10.0f, 20.0f, 5.0f);
    Plane plane(normal, inputPoint);

    Vector3 pointOnPlane = plane.getPointOnPlane();

    // Point should lie on the plane (signed distance should be zero)
    REQUIRE(plane.getSignedDistance(pointOnPlane) == Approx(0.0f));
  }

  SECTION("getDistance() Method - Always Positive")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 point(0.0f, 0.0f, 0.0f);
    Plane plane(normal, point);

    // Test points on both sides of the plane
    Vector3 positivePoint(5.0f, 0.0f, 0.0f);
    Vector3 negativePoint(-3.0f, 0.0f, 0.0f);

    REQUIRE(plane.getDistance(positivePoint) == Approx(5.0f));
    REQUIRE(plane.getDistance(negativePoint) == Approx(3.0f));

    // Both should be positive
    REQUIRE(plane.getDistance(positivePoint) > 0.0f);
    REQUIRE(plane.getDistance(negativePoint) > 0.0f);
  }
}

TEST_CASE("PLANE PERFORMANCE AND PRECISION")
{
  SECTION("High Precision Distance Calculations")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 point(0.0f, 0.0f, 0.0f);
    Plane plane(normal, point);

    // Test very precise distances
    Vector3 testPoint(1e-7f, 0.0f, 0.0f);
    float32 distance = plane.getSignedDistance(testPoint);

    REQUIRE(distance == Approx(1e-7f).margin(1e-9f));
  }

  SECTION("Large Coordinate Values")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 point(1e6f, 0.0f, 0.0f);
    Plane plane(normal, point);

    Vector3 testPoint(1e6f + 100.0f, 0.0f, 0.0f);
    float32 distance = plane.getSignedDistance(testPoint);

    REQUIRE(distance == Approx(100.0f));
    REQUIRE(std::isfinite(distance));
  }
}

TEST_CASE("PLANE MATHEMATICAL PROPERTIES")
{
  SECTION("Normal Vector Always Unit Length")
  {
    // Test various input normals
    std::vector<Vector3> testNormals = {
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        Vector3(100.0f, 200.0f, 300.0f),
        Vector3(0.001f, 0.002f, 0.003f)};

    for (const auto &testNormal : testNormals)
    {
      Vector3 point(0.0f, 0.0f, 0.0f);
      Plane plane(testNormal, point);

      Vector3 normal = plane.getNormal();
      float32 length = Vector3::Length(normal);
      REQUIRE(length == Approx(1.0f).margin(1e-5f));
    }
  }

  SECTION("Plane Equation Consistency")
  {
    Vector3 normal(2.0f, 3.0f, 4.0f);
    Vector3 point(1.0f, 2.0f, 3.0f);
    Plane plane(normal, point);

    // Test that the input point lies on the plane
    REQUIRE(plane.getSignedDistance(point) == Approx(0.0f));

    // Test that points along the normal have predictable distances
    Vector3 alongNormal = point + plane.getNormal() * 5.0f;
    REQUIRE(plane.getSignedDistance(alongNormal) == Approx(5.0f));

    Vector3 againstNormal = point - plane.getNormal() * 3.0f;
    REQUIRE(plane.getSignedDistance(againstNormal) == Approx(-3.0f));
  }
}

TEST_CASE("PLANE EDGE CASE STRESS TESTS")
{
  SECTION("Identical Points in Three-Point Constructor")
  {
    Vector3 point(1.0f, 2.0f, 3.0f);

    // All three points are the same
    Plane plane(point, point, point);

    REQUIRE(plane.isValid());
    REQUIRE(std::isfinite(plane.getD()));
  }

  SECTION("Nearly Collinear Points")
  {
    Vector3 pointA(0.0f, 0.0f, 0.0f);
    Vector3 pointB(1.0f, 0.0f, 0.0f);
    Vector3 pointC(2.0f, 1e-8f, 0.0f); // Nearly collinear

    Plane plane(pointA, pointB, pointC);

    // Should handle gracefully
    REQUIRE(std::isfinite(plane.getNormal().X));
    REQUIRE(std::isfinite(plane.getNormal().Y));
    REQUIRE(std::isfinite(plane.getNormal().Z));
    REQUIRE(std::isfinite(plane.getD()));
  }

  SECTION("Extreme Coordinate Values")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 extremePoint(std::numeric_limits<float>::max() / 2.0f, 0.0f, 0.0f);

    Plane plane(normal, extremePoint);

    REQUIRE(plane.isValid());
    REQUIRE(std::isfinite(plane.getD()));
  }
}
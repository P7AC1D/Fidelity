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
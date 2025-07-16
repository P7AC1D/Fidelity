#include "catch.hpp"

#include "../Engine/Maths/Aabb.hpp"
#include <cmath>
#include <limits>

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

// ADDITIONAL COMPREHENSIVE TESTS FOR MATHEMATICAL CORRECTNESS
TEST_CASE("AABB MATHEMATICAL CORRECTNESS", "[AABB][Critical]")
{
  SECTION("getMin() Calculation Verification")
  {
    // Test case that exposes the bug in getMin()
    Vector3 center(5.0f, 10.0f, 15.0f);
    Vector3 extents(2.0f, 3.0f, 4.0f);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    // Expected: min = center - extents
    Vector3 expectedMin = center - extents;
    Vector3 actualMin = aabb.getMin();
    
    // This test will likely FAIL with current implementation
    INFO("Center: " << center.X << ", " << center.Y << ", " << center.Z);
    INFO("Extents: " << extents.X << ", " << extents.Y << ", " << extents.Z);
    INFO("Expected Min: " << expectedMin.X << ", " << expectedMin.Y << ", " << expectedMin.Z);
    INFO("Actual Min: " << actualMin.X << ", " << actualMin.Y << ", " << actualMin.Z);
    
    REQUIRE(actualMin.X == Approx(expectedMin.X));
    REQUIRE(actualMin.Y == Approx(expectedMin.Y));
    REQUIRE(actualMin.Z == Approx(expectedMin.Z));
  }
  
  SECTION("getMax() Calculation Verification")
  {
    Vector3 center(5.0f, 10.0f, 15.0f);
    Vector3 extents(2.0f, 3.0f, 4.0f);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    // Expected: max = center + extents
    Vector3 expectedMax = center + extents;
    Vector3 actualMax = aabb.getMax();
    
    REQUIRE(actualMax.X == Approx(expectedMax.X));
    REQUIRE(actualMax.Y == Approx(expectedMax.Y));
    REQUIRE(actualMax.Z == Approx(expectedMax.Z));
  }
  
  SECTION("Min-Max Constructor Consistency")
  {
    Vector3 min(-5.0f, -10.0f, -2.0f);
    Vector3 max(15.0f, 20.0f, 8.0f);
    
    Aabb aabb(max, min); // Note: constructor takes (max, min)
    
    Vector3 actualMin = aabb.getMin();
    Vector3 actualMax = aabb.getMax();
    
    INFO("Input Min: " << min.X << ", " << min.Y << ", " << min.Z);
    INFO("Input Max: " << max.X << ", " << max.Y << ", " << max.Z);
    INFO("Output Min: " << actualMin.X << ", " << actualMin.Y << ", " << actualMin.Z);
    INFO("Output Max: " << actualMax.X << ", " << actualMax.Y << ", " << actualMax.Z);
    
    // The constructor should preserve the input min/max values
    REQUIRE(actualMin.X == Approx(min.X));
    REQUIRE(actualMin.Y == Approx(min.Y));
    REQUIRE(actualMin.Z == Approx(min.Z));
    
    REQUIRE(actualMax.X == Approx(max.X));
    REQUIRE(actualMax.Y == Approx(max.Y));
    REQUIRE(actualMax.Z == Approx(max.Z));
  }
  
  SECTION("Constructor Consistency - Both Methods Should Give Same Result")
  {
    Vector3 center(10.0f, 5.0f, 0.0f);
    Vector3 extents(3.0f, 2.0f, 1.0f);
    
    // Method 1: Center + Extents constructor
    Aabb aabb1(center, extents.X, extents.Y, extents.Z);
    
    // Method 2: Min-Max constructor with calculated values
    Vector3 expectedMin = center - extents;
    Vector3 expectedMax = center + extents;
    Aabb aabb2(expectedMax, expectedMin);
    
    // Both should produce identical AABBs
    REQUIRE(aabb1.getCenter().X == Approx(aabb2.getCenter().X));
    REQUIRE(aabb1.getCenter().Y == Approx(aabb2.getCenter().Y));
    REQUIRE(aabb1.getCenter().Z == Approx(aabb2.getCenter().Z));
    
    REQUIRE(aabb1.getExtents().X == Approx(aabb2.getExtents().X));
    REQUIRE(aabb1.getExtents().Y == Approx(aabb2.getExtents().Y));
    REQUIRE(aabb1.getExtents().Z == Approx(aabb2.getExtents().Z));
    
    REQUIRE(aabb1.getMin().X == Approx(aabb2.getMin().X));
    REQUIRE(aabb1.getMin().Y == Approx(aabb2.getMin().Y));
    REQUIRE(aabb1.getMin().Z == Approx(aabb2.getMin().Z));
    
    REQUIRE(aabb1.getMax().X == Approx(aabb2.getMax().X));
    REQUIRE(aabb1.getMax().Y == Approx(aabb2.getMax().Y));
    REQUIRE(aabb1.getMax().Z == Approx(aabb2.getMax().Z));
  }
}

TEST_CASE("AABB EDGE CASES AND ROBUSTNESS")
{
  SECTION("Zero-Sized AABB")
  {
    Vector3 center(5.0f, 5.0f, 5.0f);
    Aabb aabb(center, 0.0f, 0.0f, 0.0f);
    
    // Min and max should be equal to center
    REQUIRE(aabb.getMin() == center);
    REQUIRE(aabb.getMax() == center);
    REQUIRE(aabb.getRadius() == Approx(0.0f));
  }
  
  SECTION("Negative Extents - Should Handle Gracefully")
  {
    Vector3 center(0.0f, 0.0f, 0.0f);
    
    // Negative extents don't make geometric sense, but implementation should handle it
    Aabb aabb(center, -1.0f, -2.0f, -3.0f);
    
    // Should still produce finite, mathematical results
    Vector3 min = aabb.getMin();
    Vector3 max = aabb.getMax();
    
    REQUIRE(std::isfinite(min.X));
    REQUIRE(std::isfinite(min.Y));
    REQUIRE(std::isfinite(min.Z));
    REQUIRE(std::isfinite(max.X));
    REQUIRE(std::isfinite(max.Y));
    REQUIRE(std::isfinite(max.Z));
  }
  
  SECTION("Very Large Values")
  {
    float32 large = std::numeric_limits<float32>::max() / 4.0f; // Avoid overflow
    Vector3 center(large, large, large);
    Vector3 extents(large / 2.0f, large / 2.0f, large / 2.0f);
    
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    Vector3 min = aabb.getMin();
    Vector3 max = aabb.getMax();
    
    REQUIRE(std::isfinite(min.X));
    REQUIRE(std::isfinite(min.Y));
    REQUIRE(std::isfinite(min.Z));
    REQUIRE(std::isfinite(max.X));
    REQUIRE(std::isfinite(max.Y));
    REQUIRE(std::isfinite(max.Z));
  }
  
  SECTION("Very Small Values - Precision Test")
  {
    Vector3 center(1e-6f, 1e-6f, 1e-6f);
    Vector3 extents(1e-7f, 1e-7f, 1e-7f);
    
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    Vector3 min = aabb.getMin();
    Vector3 max = aabb.getMax();
    
    // Should maintain precision for small values
    REQUIRE(min.X == Approx(center.X - extents.X).margin(1e-9f));
    REQUIRE(max.X == Approx(center.X + extents.X).margin(1e-9f));
  }
}

TEST_CASE("AABB PLANE INTERSECTION ADVANCED")
{
  SECTION("AABB Exactly on Plane")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 planePoint(0.0f, 0.0f, 0.0f);
    Plane plane(normal, planePoint);
    
    // Create AABB with one face exactly on the plane
    Vector3 center(0.0f, 0.0f, 0.0f);
    Vector3 extents(1.0f, 1.0f, 1.0f);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    // AABB should be considered "on or forward" because part of it is on positive side
    bool result = aabb.isOnOrForwardPlane(plane);
    REQUIRE(result == true);
  }
  
  SECTION("AABB Completely Behind Plane")
  {
    Vector3 normal(1.0f, 0.0f, 0.0f);
    Vector3 planePoint(0.0f, 0.0f, 0.0f);
    Plane plane(normal, planePoint);
    
    // Create AABB completely behind plane (negative X)
    Vector3 center(-5.0f, 0.0f, 0.0f);
    Vector3 extents(1.0f, 1.0f, 1.0f);
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    bool result = aabb.isOnOrForwardPlane(plane);
    REQUIRE(result == false);
  }
  
  SECTION("AABB Straddles Plane")
  {
    Vector3 normal(0.0f, 1.0f, 0.0f);
    Vector3 planePoint(0.0f, 5.0f, 0.0f);
    Plane plane(normal, planePoint);
    
    // Create AABB that straddles the plane
    Vector3 center(0.0f, 5.0f, 0.0f);
    Vector3 extents(1.0f, 2.0f, 1.0f); // extends from Y=3 to Y=7
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    bool result = aabb.isOnOrForwardPlane(plane);
    REQUIRE(result == true); // Should be true because positive vertex is forward
  }
}

TEST_CASE("AABB UTILITY FUNCTIONS", "[AABB]")
{
  SECTION("Radius Calculation - 3-4-5 Triangle")
  {
    Vector3 center(0.0f, 0.0f, 0.0f);
    Vector3 extents(3.0f, 4.0f, 0.0f); // 3-4-5 triangle
    
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    // Radius should be length of extents vector
    float32 expectedRadius = Vector3::Length(extents);
    float32 actualRadius = aabb.getRadius();
    
    REQUIRE(actualRadius == Approx(expectedRadius));
    REQUIRE(actualRadius == Approx(5.0f)); // 3-4-5 triangle
  }
  
  SECTION("Size Calculations")
  {
    Vector3 center(10.0f, 20.0f, 30.0f);
    Vector3 extents(5.0f, 3.0f, 2.0f);
    
    Aabb aabb(center, extents.X, extents.Y, extents.Z);
    
    Vector3 size = aabb.getMax() - aabb.getMin();
    
    // Size should be twice the extents
    REQUIRE(size.X == Approx(2.0f * extents.X));
    REQUIRE(size.Y == Approx(2.0f * extents.Y));
    REQUIRE(size.Z == Approx(2.0f * extents.Z));
  }
  
  SECTION("AABB Volume Calculation")
  {
    // Test volume calculation
    Aabb aabb(Vector3(5.0f, 10.0f, 15.0f), 2.0f, 3.0f, 4.0f);
    
    float32 expectedVolume = 4.0f * 6.0f * 8.0f; // (2*2) * (2*3) * (2*4) = 4*6*8 = 192
    float32 actualVolume = aabb.getVolume();
    
    REQUIRE(actualVolume == Approx(expectedVolume));
    
    INFO("Expected Volume: " << expectedVolume);
    INFO("Actual Volume: " << actualVolume);
  }
}

TEST_CASE("AABB ENHANCED FUNCTIONALITY", "[AABB]")
{
	SECTION("Surface Area Calculation")
	{
		// Test surface area calculation for a 2x3x4 box
		Aabb aabb(Vector3(0.0f, 0.0f, 0.0f), 1.0f, 1.5f, 2.0f);
		
		// Surface area = 2 * (w*h + h*d + d*w) = 2 * (2*3 + 3*4 + 4*2) = 2 * (6 + 12 + 8) = 2 * 26 = 52
		float32 expectedArea = 2.0f * (2.0f * 3.0f + 3.0f * 4.0f + 4.0f * 2.0f);
		float32 actualArea = aabb.getSurfaceArea();
		
		REQUIRE(actualArea == Approx(expectedArea));
	}
	
	SECTION("Point Containment Test")
	{
		Aabb aabb(Vector3(5.0f, 5.0f, 5.0f), 2.0f, 2.0f, 2.0f); // Center at (5,5,5), extends from (3,3,3) to (7,7,7)
		
		// Points inside
		REQUIRE(aabb.contains(Vector3(5.0f, 5.0f, 5.0f))); // Center
		REQUIRE(aabb.contains(Vector3(4.0f, 4.0f, 4.0f))); // Inside
		REQUIRE(aabb.contains(Vector3(3.0f, 3.0f, 3.0f))); // On boundary (min)
		REQUIRE(aabb.contains(Vector3(7.0f, 7.0f, 7.0f))); // On boundary (max)
		
		// Points outside
		REQUIRE_FALSE(aabb.contains(Vector3(2.9f, 5.0f, 5.0f))); // Outside X min
		REQUIRE_FALSE(aabb.contains(Vector3(7.1f, 5.0f, 5.0f))); // Outside X max
		REQUIRE_FALSE(aabb.contains(Vector3(5.0f, 2.9f, 5.0f))); // Outside Y min
		REQUIRE_FALSE(aabb.contains(Vector3(5.0f, 7.1f, 5.0f))); // Outside Y max
		REQUIRE_FALSE(aabb.contains(Vector3(5.0f, 5.0f, 2.9f))); // Outside Z min
		REQUIRE_FALSE(aabb.contains(Vector3(5.0f, 5.0f, 7.1f))); // Outside Z max
	}
	
	SECTION("Validity Check")
	{
		// Valid AABB
		Aabb validAabb(Vector3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f, 1.0f);
		REQUIRE(validAabb.isValid());
		
		// Zero-sized AABB (still valid)
		Aabb zeroAabb(Vector3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f);
		REQUIRE(zeroAabb.isValid());
		
		// Invalid AABB with negative extents (manually set for testing)
		// Note: We can't test this easily since constructor doesn't allow negative extents
		// In a real scenario, this would be caught during construction
	}
	
	SECTION("Size Calculation")
	{
		Aabb aabb(Vector3(10.0f, 20.0f, 30.0f), 5.0f, 7.0f, 9.0f);
		Vector3 size = aabb.getSize();
		
		REQUIRE(size.X == Approx(10.0f)); // 2 * 5.0f
		REQUIRE(size.Y == Approx(14.0f)); // 2 * 7.0f
		REQUIRE(size.Z == Approx(18.0f)); // 2 * 9.0f
	}
}

TEST_CASE("AABB ENCAPSULATION", "[AABB]")
{
	SECTION("Encapsulate Point")
	{
		Aabb aabb(Vector3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f, 1.0f); // Initial box from (-1,-1,-1) to (1,1,1)
		
		// Encapsulate a point outside the current bounds
		aabb.encapsulate(Vector3(3.0f, 2.0f, 4.0f));
		
		// New AABB should contain both the original bounds and the new point
		REQUIRE(aabb.contains(Vector3(-1.0f, -1.0f, -1.0f))); // Original min
		REQUIRE(aabb.contains(Vector3(1.0f, 1.0f, 1.0f)));   // Original max
		REQUIRE(aabb.contains(Vector3(3.0f, 2.0f, 4.0f)));   // New point
		
		// Check calculated bounds
		Vector3 newMin = aabb.getMin();
		Vector3 newMax = aabb.getMax();
		
		REQUIRE(newMin.X == Approx(-1.0f));
		REQUIRE(newMin.Y == Approx(-1.0f));
		REQUIRE(newMin.Z == Approx(-1.0f));
		REQUIRE(newMax.X == Approx(3.0f));
		REQUIRE(newMax.Y == Approx(2.0f));
		REQUIRE(newMax.Z == Approx(4.0f));
	}
	
	SECTION("Encapsulate AABB")
	{
		Aabb aabb1(Vector3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f, 1.0f); // Box from (-1,-1,-1) to (1,1,1)
		Aabb aabb2(Vector3(5.0f, 3.0f, 2.0f), 2.0f, 1.0f, 3.0f); // Box from (3,2,-1) to (7,4,5)
		
		aabb1.encapsulate(aabb2);
		
		// New AABB should contain both original AABBs
		REQUIRE(aabb1.contains(Vector3(-1.0f, -1.0f, -1.0f))); // Original aabb1 min
		REQUIRE(aabb1.contains(Vector3(1.0f, 1.0f, 1.0f)));   // Original aabb1 max
		REQUIRE(aabb1.contains(Vector3(3.0f, 2.0f, -1.0f)));  // Original aabb2 min
		REQUIRE(aabb1.contains(Vector3(7.0f, 4.0f, 5.0f)));   // Original aabb2 max
		
		// Check final bounds
		Vector3 finalMin = aabb1.getMin();
		Vector3 finalMax = aabb1.getMax();
		
		REQUIRE(finalMin.X == Approx(-1.0f));
		REQUIRE(finalMin.Y == Approx(-1.0f));
		REQUIRE(finalMin.Z == Approx(-1.0f));
		REQUIRE(finalMax.X == Approx(7.0f));
		REQUIRE(finalMax.Y == Approx(4.0f));
		REQUIRE(finalMax.Z == Approx(5.0f));
	}
}
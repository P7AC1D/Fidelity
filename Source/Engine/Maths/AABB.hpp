#pragma once
#include "Matrix4.hpp"
#include "Vector3.hpp"

class Plane;

class Aabb
{
public:
	/// @brief Constructs an AABB with default values.
	Aabb();

	/// @brief Constructs an AABB from a maximum and minimum extents.
	/// @param max The extents in the postive x,y and z direction.
	/// @param min The extents in the negative x,y and z direction.
	Aabb(const Vector3 &max, const Vector3 &min);
	/// @brief Constructs an AABB from a center vector and x,y and z extents.
	/// @param center A vector to the center of the AABB.
	/// @param xExtents The extents in the x direction from the center of the AABB.
	/// @param yExtents The extents in the y direction from the center of the AABB.
	/// @param zExtents The extents in the z direction from the center of the AABB.
	Aabb(const Vector3 &center, float32 xExtents, float32 yExtents, float32 zExtents);

	Vector3 getCenter() const { return _center; }
	Vector3 getExtents() const { return _extents; }

	/// @brief Calculates and returns the minimum extents in the postive x,y and z directions.
	/// @return The minimum extents.
	Vector3 getMin() const;
	/// @brief Calculates and returns the maximum extents in the postive x,y and z directions.
	/// @return The maximum extents.
	Vector3 getMax() const;

	/// @brief Treats the AABB as a spherical bounding volume and calculates a radius based on the extents.
	/// @return The radius of bounding sphere around the AABB.
	float32 getRadius() const;

	/// @brief Gets the size (width, height, depth) of the AABB.
	/// @return Vector containing the dimensions along each axis.
	Vector3 getSize() const { return _extents * 2.0f; }

	/// @brief Calculates the volume of the AABB.
	/// @return The volume of the bounding box.
	float32 getVolume() const;

	/// @brief Calculates the surface area of the AABB.
	/// @return The surface area of the bounding box.
	float32 getSurfaceArea() const;

	/// @brief Checks if this AABB is valid (non-negative extents).
	/// @return True if the AABB has valid dimensions.
	bool isValid() const { return _extents.X >= 0.0f && _extents.Y >= 0.0f && _extents.Z >= 0.0f; }

	/// @brief Checks if a point is contained within this AABB.
	/// @param point The point to test.
	/// @return True if the point is inside or on the boundary of the AABB.
	bool contains(const Vector3& point) const;

	/// @brief Expands this AABB to include another AABB.
	/// @param other The AABB to include.
	void encapsulate(const Aabb& other);

	/// @brief Expands this AABB to include a point.
	/// @param point The point to include.
	void encapsulate(const Vector3& point);

	/// @brief Test if the AABB is positioned ahead or behind the plane.
	/// @param plane The plane to test against.
	/// @return True of the AABB is infront of the plane. False otherwise.
	bool isOnOrForwardPlane(const Plane &plane) const;

private:
	Vector3 getMaxPositivePointFromPlane(const Plane &plane) const;
	Vector3 getMaxNegativePointFromPlane(const Plane &plane) const;

	Vector3 _center;
	Vector3 _extents;
};
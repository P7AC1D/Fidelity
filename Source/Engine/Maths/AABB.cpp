#include "AABB.hpp"

#include "Plane.hpp"
#include "Vector4.hpp"

#include <cmath>

Aabb::Aabb() : _center(0.f),
							 _extents(1.f)
{
}

Aabb::Aabb(const Vector3 &max, const Vector3 &min) : _center((max + min) * 0.5f),
																										 _extents(max.X - _center.X, max.Y - _center.Y, max.Z - _center.Z)
{
}

Aabb::Aabb(const Vector3 &center, float32 xExtents, float32 yExtents, float32 zExtents) : _center(center),
																																													_extents(xExtents, yExtents, zExtents)
{
}

Vector3 Aabb::getMin() const
{
	Vector3 max(getMax());
	return _center * 2.0f - max;
}

Vector3 Aabb::getMax() const
{
	return _extents + _center;
}

float32 Aabb::getRadius() const
{
	return Vector3::Length(_extents);
}

bool Aabb::isOnOrForwardPlane(const Plane &plane) const
{
	if (plane.getSignedDistance(getMaxPositivePointFromPlane(plane)) <= 0.0f)
	{
		return false;
	}
	return true;
}

/// @brief Calculates and returns one of the eight corners of the AABB which is furthest from the plane along the direction of the normal.
/// @param plane The plane to test against.
/// @return Coordinates of the furthest corner.
Vector3 Aabb::getMaxPositivePointFromPlane(const Plane &plane) const
{
	Vector3 max(getMax());
	Vector3 min(getMin());
	Vector3 p(min);
	Vector3 normal(plane.getNormal());

	if (normal.X >= 0.0f)
	{
		p.X = max.X;
	}

	if (normal.Y >= 0.0f)
	{
		p.Y = max.Y;
	}

	if (normal.Z >= 0.0f)
	{
		p.Z = max.Z;
	}
	return p;
}

/// @brief Calculates and returns one of the eight corners of the AABB which is furthest from the plane along the oposite direction of the normal.
/// @param plane The plane to test against.
/// @return Coordinates of the furthest corner.
Vector3 Aabb::getMaxNegativePointFromPlane(const Plane &plane) const
{
	Vector3 max(getMax());
	Vector3 min(getMin());
	Vector3 n(max);
	Vector3 normal(plane.getNormal());

	if (normal.X >= 0.0f)
	{
		n.X = min.X;
	}

	if (normal.Y >= 0.0f)
	{
		n.Y = min.Y;
	}

	if (normal.Z >= 0.0f)
	{
		n.Z = min.Z;
	}
	return n;
}
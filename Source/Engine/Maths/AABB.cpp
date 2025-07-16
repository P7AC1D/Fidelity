#include "AABB.hpp"

#include "Plane.hpp"
#include "Vector4.hpp"

#include <cmath>
#include <algorithm>

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
	return _center - _extents;
}

Vector3 Aabb::getMax() const
{
	return _extents + _center;
}

float32 Aabb::getRadius() const
{
	// Optimized: Calculate magnitude directly instead of calling Vector3::Length
	return sqrtf(_extents.X * _extents.X + _extents.Y * _extents.Y + _extents.Z * _extents.Z);
}

float32 Aabb::getVolume() const
{
	Vector3 size = getSize();
	return size.X * size.Y * size.Z;
}

float32 Aabb::getSurfaceArea() const
{
	Vector3 size = getSize();
	return 2.0f * (size.X * size.Y + size.Y * size.Z + size.Z * size.X);
}

bool Aabb::contains(const Vector3& point) const
{
	Vector3 min = getMin();
	Vector3 max = getMax();
	return (point.X >= min.X && point.X <= max.X &&
			point.Y >= min.Y && point.Y <= max.Y &&
			point.Z >= min.Z && point.Z <= max.Z);
}

void Aabb::encapsulate(const Aabb& other)
{
	Vector3 thisMin = getMin();
	Vector3 thisMax = getMax();
	Vector3 otherMin = other.getMin();
	Vector3 otherMax = other.getMax();
	
	Vector3 newMin(
		std::min(thisMin.X, otherMin.X),
		std::min(thisMin.Y, otherMin.Y),
		std::min(thisMin.Z, otherMin.Z)
	);
	
	Vector3 newMax(
		std::max(thisMax.X, otherMax.X),
		std::max(thisMax.Y, otherMax.Y),
		std::max(thisMax.Z, otherMax.Z)
	);
	
	_center = (newMin + newMax) * 0.5f;
	_extents = (newMax - newMin) * 0.5f;
}

void Aabb::encapsulate(const Vector3& point)
{
	Vector3 currentMin = getMin();
	Vector3 currentMax = getMax();
	
	Vector3 newMin(
		std::min(currentMin.X, point.X),
		std::min(currentMin.Y, point.Y),
		std::min(currentMin.Z, point.Z)
	);
	
	Vector3 newMax(
		std::max(currentMax.X, point.X),
		std::max(currentMax.Y, point.Y),
		std::max(currentMax.Z, point.Z)
	);
	
	_center = (newMin + newMax) * 0.5f;
	_extents = (newMax - newMin) * 0.5f;
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
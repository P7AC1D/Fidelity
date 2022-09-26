#include "AABB.hpp"

#include "Plane.hpp"
#include "Vector4.hpp"

Aabb::Aabb()
{
}

Aabb::Aabb(const Vector3 &posBounds, const Vector3 &negBounds) : _posBounds(posBounds), _negBounds(negBounds)
{
}

Vector3 Aabb::GetCenter() const
{
	return Vector3((_posBounds.X + _negBounds.X) / 2.0f, (_posBounds.Y + _negBounds.Y) / 2.0f, (_posBounds.Z + _negBounds.Z) / 2.0f);
}

Vector3 Aabb::GetSize() const
{
	return _posBounds - _negBounds;
}

Vector3 Aabb::GetHalfSize() const
{
	return GetSize() * 0.5f;
}

float32 Aabb::GetRadius() const
{
	return Vector3::Length(GetHalfSize());
}

bool Aabb::isInFrustrumPlane(const Plane &plane) const
{
	const Vector3 planeNormal(plane.getNormal());
	const Vector3 extents(GetHalfSize());

	const float32 r = extents.X * std::abs(planeNormal.X) +
										extents.Y * std::abs(planeNormal.Y) + extents.Z * std::abs(planeNormal.Z);

	return -r <= plane.getSignedDistance(GetCenter());
}
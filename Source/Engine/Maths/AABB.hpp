#pragma once
#include "Matrix4.hpp"
#include "Vector3.hpp"

class Plane;

class Aabb
{
public:
	Aabb();
	Aabb(const Vector3 &posBounds, const Vector3 &negBounds);

	Vector3 GetCenter() const;
	Vector3 GetSize() const;
	Vector3 GetHalfSize() const;
	Vector3 GetPosBounds() const { return _posBounds; }
	Vector3 GetNegBounds() const { return _negBounds; }

	float32 GetRadius() const;

	bool isInFrustrumPlane(const Plane &plane) const;

private:
	Vector3 _posBounds;
	Vector3 _negBounds;
};
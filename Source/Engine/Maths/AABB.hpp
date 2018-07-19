#pragma once
#include "Matrix4.hpp"
#include "Vector3.hpp"

class Aabb
{
public:
	Aabb();
	Aabb(const Vector3& posBounds, const Vector3& negBounds);

	Vector3 GetCenter() const;
	Vector3 GetSize() const;
	Vector3 GetHalfSize() const;

	float32 GetRadius() const;

private:
	Vector3 _posBounds;
	Vector3 _negBounds;
};
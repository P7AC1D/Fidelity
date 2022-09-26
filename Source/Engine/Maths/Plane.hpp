#pragma once
#include "../Core/Types.hpp"
#include "Vector3.hpp"

class Plane
{
public:
	Plane();
	Plane(const Plane &copy);
	Plane(const Vector3 &normal, const Vector3 &point);
	Plane(const Vector3 &normal, float32 d);
	Plane(const Vector3 &pointA, const Vector3 &pointB, const Vector3 &pointC);

	Vector3 getNormal() const { return _normal; }
	float32 getDistance() const { return _d; }
	float32 getSignedDistance(const Vector3 &point) const;

private:
	Vector3 _normal;
	float32 _d;
};
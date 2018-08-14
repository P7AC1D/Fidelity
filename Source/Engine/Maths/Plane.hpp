#pragma once
#include "../Core/Types.hpp"
#include "Vector3.hpp"

class Plane
{
public:
	Plane();
	Plane(const Plane& copy);
	Plane(const Vector3& normal, const Vector3& point);
	Plane(const Vector3& normal, float32 d);
	Plane(const Vector3& pointA, const Vector3& pointB, const Vector3& pointC);

public:
	Vector3 Normal;
	float32 D;
};
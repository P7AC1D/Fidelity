#include "Plane.hpp"

Plane::Plane() : Normal(Vector3::Zero), D(0.0f)
{
}

Plane::Plane(const Plane& copy) : Normal(copy.Normal), D(copy.D)
{
}

Plane::Plane(const Vector3& normal, const Vector3& point) : Normal(normal), D(Vector3::Dot(normal, point))
{
}

Plane::Plane(const Vector3& normal, float32 d) : Normal(normal), D(d)
{
}

Plane::Plane(const Vector3& pointA, const Vector3& pointB, const Vector3& pointC)
{
	Vector3 edgeA = pointA - pointB;
	Vector3 edgeB = pointC - pointA;
	Normal = Vector3::Normalize(Vector3::Cross(edgeA, edgeB));
	D = Vector3::Dot(Normal, pointA);
}
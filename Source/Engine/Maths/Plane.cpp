#include "Plane.hpp"

Plane::Plane() : _normal(Vector3::Zero), _d(0.0f)
{
}

Plane::Plane(const Plane &copy) : _normal(copy._normal), _d(copy._d)
{
}

Plane::Plane(const Vector3 &normal, const Vector3 &point) : _normal(normal), _d(Vector3::Dot(normal, point))
{
}

Plane::Plane(const Vector3 &normal, float32 d) : _normal(normal), _d(d)
{
}

Plane::Plane(const Vector3 &pointA, const Vector3 &pointB, const Vector3 &pointC)
{
	Vector3 edgeA = pointA - pointB;
	Vector3 edgeB = pointC - pointA;
	_normal = Vector3::Normalize(Vector3::Cross(edgeA, edgeB));
	_d = Vector3::Dot(_normal, pointA);
}

float32 Plane::getSignedDistance(const Vector3 &point) const
{
	return Vector3::Dot(_normal, point) - _d;
}
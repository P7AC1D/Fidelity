#include "Plane.hpp"

Plane::Plane() : _normal(Vector3::Identity), _d(0.0f)
{
}

Plane::Plane(const Vector3 &normal, const Vector3 &point) : _normal(Vector3::Normalize(normal)), _d(-Vector3::Dot(_normal, point))
{
}

Plane::Plane(const Vector3 &pointA, const Vector3 &pointB, const Vector3 &pointC)
{
	Vector3 edgeA = pointA - pointB;
	Vector3 edgeB = pointC - pointA;
	_normal = Vector3::Normalize(Vector3::Cross(edgeA, edgeB));
	_d = -Vector3::Dot(_normal, pointA);
}

float32 Plane::getSignedDistance(const Vector3 &point) const
{
	return Vector3::Dot(_normal, point) + _d;
}

void Plane::normalize()
{
	float32 length = Vector3::Length(_normal);
	_normal /= length;
	_d /= length;
}
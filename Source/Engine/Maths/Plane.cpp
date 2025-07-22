#include "Plane.hpp"
#include <cmath>

Plane::Plane() : _normal(Vector3::Identity), _d(0.0f)
{
}

Plane::Plane(const Vector3 &normal, const Vector3 &point)
{
  float32 length = Vector3::Length(normal);
  if (length > 1e-8f)
  {
    _normal = normal / length;
    _d = -Vector3::Dot(_normal, point);
  }
  else
  {
    // Degenerate case: set to a default plane (e.g., XY plane at origin)
    _normal = Vector3(0.0f, 0.0f, 1.0f);
    _d = 0.0f;
  }
}

Plane::Plane(const Vector3 &pointA, const Vector3 &pointB, const Vector3 &pointC)
{
  Vector3 edgeA = pointA - pointB;
  Vector3 edgeB = pointC - pointA;
  Vector3 crossProduct = Vector3::Cross(edgeA, edgeB);

  float32 length = Vector3::Length(crossProduct);
  if (length > 1e-8f)
  {
    _normal = crossProduct / length;
    _d = -Vector3::Dot(_normal, pointA);
  }
  else
  {
    // Collinear points: create a degenerate plane
    _normal = Vector3(0.0f, 0.0f, 1.0f);
    _d = 0.0f;
  }
}

float32 Plane::getSignedDistance(const Vector3 &point) const
{
  return Vector3::Dot(_normal, point) + _d;
}

void Plane::normalize()
{
  float32 length = Vector3::Length(_normal);
  if (length > 1e-8f) // Avoid division by zero
  {
    _normal /= length;
    _d /= length;
  }
  // If length is too small, keep current values (degenerate plane)
}
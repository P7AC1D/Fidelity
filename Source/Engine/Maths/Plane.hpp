#pragma once
#include "../Core/Types.hpp"
#include "Vector3.hpp"
#include <cmath>

class Plane
{
public:
  Plane();
  Plane(const Vector3 &normal, const Vector3 &point);
  Plane(const Vector3 &pointA, const Vector3 &pointB, const Vector3 &pointC);

  Vector3 getNormal() const { return _normal; }
  float32 getD() const { return _d; }
  float32 getSignedDistance(const Vector3 &point) const;

  void normalize();

  // Utility methods
  bool isValid() const { return Vector3::Length(_normal) > 1e-8f; }
  Vector3 getPointOnPlane() const { return _normal * (-_d); }

  // Distance to point (always positive)
  float32 getDistance(const Vector3 &point) const { return std::abs(getSignedDistance(point)); }

private:
  Vector3 _normal;
  float32 _d;
};
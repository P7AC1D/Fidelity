#pragma once
#include "Vector3.hpp"

class Aabb;

class Ray
{
public:
  Ray(const Vector3& position, const Vector3& direction);
  
  bool Intersects(const Aabb& aabb) const;
  
private:
  Vector3 _position;
  Vector3 _direction;
  Vector3 _directionInvs;
};

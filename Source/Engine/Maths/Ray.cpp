#include "Ray.hpp"

#include <cmath>
#include <numeric>
#include <vector>

#include "AABB.hpp"
#include "Plane.hpp"

Ray::Ray(const Vector3 &position, const Vector3 &direction) : _position(position),
                                                              _direction(Vector3::Normalize(direction))
{
  _directionInvs = 1.0f / _direction;
}

bool Ray::Intersects(const Aabb &aabb) const
{
  float32 tmin = 0.0f, tmax = std::numeric_limits<float32>::max();
  Vector3 min = aabb.getMin(), max = aabb.getMax();

  // Case when ray origin is inside AABB.
  if (min.X < _position.X && min.Y < _position.Y && min.Z < _position.Z &&
      max.X > _position.X && max.Y > _position.Y && max.Z > _position.Z)
  {
    return false;
  }

  // Fast slab method from here: https://tavianator.com/2022/ray_box_boundary.html
  for (int i = 0; i < 3; ++i)
  {
    float t1 = (min[i] - _position[i]) * _directionInvs[i];
    float t2 = (max[i] - _position[i]) * _directionInvs[i];

    tmin = std::fmaxf(tmin, std::fminf(t1, t2));
    tmax = std::fminf(tmax, std::fmaxf(t1, t2));
  }

  return tmin < tmax;
}

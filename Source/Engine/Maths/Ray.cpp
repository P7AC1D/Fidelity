#include "Ray.hpp"

#include <vector>
#include "AABB.hpp"
#include "Plane.hpp"

Ray::Ray(const Vector3& position, const Vector3& direction) :
_position(position),
_direction(direction)
{
}

bool Ray::Intersects(const Aabb &aabb) const
{
  Vector3 size(aabb.GetSize());
  std::vector<Plane> planes;
  planes.reserve(6);
  
  planes.emplace_back(Vector3(size.X, size.Y, size.Z), Vector3(size.X, size.Y, -size.Z), Vector3(size.X, -size.Y, -size.Z));
  planes.emplace_back(Vector3(-size.X, size.Y, size.Z), Vector3(-size.X, size.Y, -size.Z), Vector3(-size.X, -size.Y, -size.Z));
  planes.emplace_back(Vector3(-size.X, size.Y, size.Z), Vector3(size.X, size.Y, size.Z), Vector3(size.X, -size.Y, size.Z));
  planes.emplace_back(Vector3(size.X, size.Y, -size.Z), Vector3(-size.X, size.Y, -size.Z), Vector3(-size.X, -size.Y, -size.Z));
  planes.emplace_back(Vector3(-size.X, size.Y, -size.Z), Vector3(size.X, size.Y, size.Z), Vector3(size.X, -size.Y, size.Z));
  planes.emplace_back(Vector3(-size.X, -size.Y, size.Z), Vector3(size.X, -size.Y, size.Z), Vector3(size.X, -size.Y, -size.Z));
  
  for (auto& plane : planes)
  {
    float32 t = - (Vector3::Dot(_position, plane.Normal) + plane.D) / (Vector3::Dot(_direction, plane.Normal));
    if (t > 0.0f)
    {
      return true;
    }
  }
  return false;
}
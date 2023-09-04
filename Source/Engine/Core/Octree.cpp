#include "Octree.h"

#include "../Maths/AABB.hpp"

void Octree::build(const Aabb& sceneBounds, uint32 depth)
{
  Vector3 extents = sceneBounds.getExtents();
  Vector3 center = sceneBounds.getCenter();


}
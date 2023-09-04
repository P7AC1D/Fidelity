#include <list>
#include <memory>

#include "Types.hpp"

class Aabb;
class OctreeNode;

class Octree
{
public:
  void build(const Aabb& sceneBounds, uint32 depth);

private:
  std::shared_ptr<OctreeNode> _rootNode;
};
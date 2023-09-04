#include <vector>

#include "Types.hpp"

class Aabb;
class GameObject;

class OctreeNode
{
public:
  void build(const Aabb& sceneBounds, const std::shared_ptr<GameObject>& parentObject);

private:
  std::vector<Aabb> _nodes;
};
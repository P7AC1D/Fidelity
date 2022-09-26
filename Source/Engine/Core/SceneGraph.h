#pragma once
#include <map>
#include <vector>

#include "Types.hpp"

class SceneGraph
{
public:
  void addNode(uint64 gameObjectId);
  void addChildToNode(uint64 parentGameObjectId, uint64 childGameObjectId);

  bool doesNodeHaveChildren(uint64 parentGameObjectId) const;

  const std::vector<uint64> &getNodeChildren(uint64 parentGameObjectId) const;

private:
  std::map<uint64, std::vector<uint64>> _sceneGraph;
};
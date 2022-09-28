#pragma once
#include <map>
#include <vector>

#include "Types.hpp"

class SceneGraph
{
public:
  void addNode(int64 gameObjectId);
  void addChildToNode(int64 parentGameObjectId, int64 childGameObjectId);

  bool doesNodeHaveChildren(int64 parentGameObjectId) const;

  const std::vector<int64> &getNodeChildren(int64 parentGameObjectId) const;

private:
  std::map<int64, std::vector<int64>> _sceneGraph;
};
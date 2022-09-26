#include "SceneGraph.h"

void SceneGraph::addNode(uint64 gameObjectId)
{
  _sceneGraph.insert(std::pair<uint64, std::vector<uint64>>(gameObjectId, {}));
}

void SceneGraph::addChildToNode(uint64 parentGameObjectId, uint64 childGameObjectId)
{
  _sceneGraph[parentGameObjectId].push_back(childGameObjectId);
}

bool SceneGraph::doesNodeHaveChildren(uint64 parentGameObjectId) const
{
  auto iter = _sceneGraph.find(parentGameObjectId);
  if (iter == _sceneGraph.end())
  {
    return false;
  }
  return !iter->second.empty();
}

const std::vector<uint64> &SceneGraph::getNodeChildren(uint64 parentGameObjectId) const
{
  auto iter = _sceneGraph.find(parentGameObjectId);
  if (iter == _sceneGraph.end())
  {
    return std::move(std::vector<uint64>());
  }
  return iter->second;
}
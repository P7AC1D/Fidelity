#include "SceneGraph.h"

void SceneGraph::addNode(int64 gameObjectId)
{
  _sceneGraph.insert(std::pair<int64, std::vector<int64>>(gameObjectId, {}));
}

void SceneGraph::addChildToNode(int64 parentGameObjectId, int64 childGameObjectId)
{
  _sceneGraph[parentGameObjectId].push_back(childGameObjectId);
}

bool SceneGraph::doesNodeHaveChildren(int64 parentGameObjectId) const
{
  auto iter = _sceneGraph.find(parentGameObjectId);
  if (iter == _sceneGraph.end())
  {
    return false;
  }
  return !iter->second.empty();
}

const std::vector<int64> &SceneGraph::getNodeChildren(int64 parentGameObjectId) const
{
  auto iter = _sceneGraph.find(parentGameObjectId);
  if (iter == _sceneGraph.end())
  {
    return std::move(std::vector<int64>());
  }
  return iter->second;
}
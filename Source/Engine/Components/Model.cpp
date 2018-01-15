#include "Model.hpp"

#include <cassert>

using namespace Components;
using namespace Rendering;

Model::Model():
Component("Model")
{
}

void Model::PushMesh(const StaticMesh& mesh)
{
  _subMeshes.emplace_back(mesh);
}

StaticMesh& Model::GetMeshAtIndex(size_t index)
{
  assert(index < _subMeshes.size());
  return _subMeshes[index];
}
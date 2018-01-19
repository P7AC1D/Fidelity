#include "Renderable.hpp"

#include <cassert>

namespace Rendering
{
void Renderable::PushMesh(const StaticMesh& mesh)
{
  _subMeshes.emplace_back(mesh);
}

StaticMesh& Renderable::GetMeshAtIndex(size_t index)
{
  assert(index < _subMeshes.size());
  return _subMeshes[index];
}
}
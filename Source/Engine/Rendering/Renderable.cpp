#include "Renderable.hpp"

#include <cassert>

#include "StaticMesh.h"

namespace Rendering
{
Renderable::Renderable():
_castShadows(true)
{
}

void Renderable::PushMesh(std::shared_ptr<StaticMesh> mesh)
{
  _subMeshes.push_back(mesh);
}

std::shared_ptr<StaticMesh> Renderable::GetMeshAtIndex(size_t index) const
{
  assert(index < _subMeshes.size());
  return _subMeshes[index];
}
}

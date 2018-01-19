#pragma once
#include <vector>

#include "StaticMesh.h"

namespace Rendering
{
class Renderable
{
public:
  void PushMesh(const StaticMesh& mesh);
  StaticMesh& GetMeshAtIndex(size_t index);

  inline size_t GetMeshCount() const { return _subMeshes.size(); }

private:
  std::vector<StaticMesh> _subMeshes;
};
}
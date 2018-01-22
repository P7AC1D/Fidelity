#pragma once
#include <vector>

#include "StaticMesh.h"

namespace Rendering
{
class Renderable
{
public:
  Renderable();

  inline void CastShadows(bool castShadows) { _castShadows = castShadows; }
  inline bool CastShadows() const { return _castShadows; }

  void PushMesh(const StaticMesh& mesh);
  StaticMesh& GetMeshAtIndex(size_t index);

  inline size_t GetMeshCount() const { return _subMeshes.size(); }

private:
  std::vector<StaticMesh> _subMeshes;
  bool _castShadows;
};
}
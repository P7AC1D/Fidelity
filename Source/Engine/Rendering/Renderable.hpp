#pragma once
#include <memory>
#include <vector>

namespace Rendering
{
class StaticMesh;

class Renderable
{
public:
  Renderable();

  inline void CastShadows(bool castShadows) { _castShadows = castShadows; }
  inline bool CastShadows() const { return _castShadows; }

  void PushMesh(std::shared_ptr<StaticMesh> mesh);
  std::shared_ptr<StaticMesh> GetMeshAtIndex(size_t index) const;

  inline size_t GetMeshCount() const { return _subMeshes.size(); }

private:
  std::vector<std::shared_ptr<StaticMesh>> _subMeshes;
  bool _castShadows;
};
}
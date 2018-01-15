#pragma once
#include <vector>

#include "../Rendering/StaticMesh.h"
#include "Component.h"

class Model : public Components::Component
{
public:
  Model();

  void PushMesh(const Rendering::StaticMesh& mesh);
  Rendering::StaticMesh& GetMeshAtIndex(size_t index);

  inline size_t GetMeshCount() const { return _subMeshes.size(); }

private:
  std::vector<Rendering::StaticMesh> _subMeshes;
};
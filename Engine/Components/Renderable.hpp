#pragma once
#include <memory>
#include <vector>

#include "Component.hpp"
#include "Types.h"

class Material;
class Mesh;

class Renderable : public Component
{
public:
  void AddMesh(const Mesh& mesh);
  
  void Update(uint32 dtMs) const override;
  
protected:
  Renderable(const GameObject& parent, uint64 id);
  
private:
  std::vector<Mesh> _meshes;
};

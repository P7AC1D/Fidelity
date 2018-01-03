#include "Renderable.hpp"

#include "../Rendering/Mesh.hpp"

void Renderable::AddMesh(const Mesh& mesh)
{
  _meshes.push_back(mesh);
}

Renderable::Renderable(const GameObject& parent, uint64 id):
Component("Renderable", parent, id)
{
}

void Renderable::Update(uint32 dtMs) const
{
  for (auto& mesh : _meshes)
  {
    mesh.Draw();
  }
}

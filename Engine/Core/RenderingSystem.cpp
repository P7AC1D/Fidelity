#include "RenderingSystem.hpp"

#include "../Components/Renderable.hpp"
#include "Material.hpp"

void RenderingSystem::RegisterMaterial(Material* material)
{
  auto iter = std::find_if(_materials.begin(), _materials.end(), [=](Material* material)
                           {
                             return material == nullptr;
                           });
  if (iter == _materials.end())
  {
    _materials.push_back(material);
  }
  else
  {
    *iter = material;
  }
}

void RenderingSystem::DeregisterMaterial(uint32 id)
{
  auto iter = std::find_if(_materials.begin(), _materials.end(), [=](Material* material)
                           {
                             return material->GetId() == id;
                           });
  *iter = nullptr;
}

void RenderingSystem::Submit(const RenderCommand &renderCommand)
{
  _renderQueue.insert(renderCommand);
}

RenderingSystem::~RenderingSystem()
{
  
}

RenderingSystem::RenderingSystem()
{
}

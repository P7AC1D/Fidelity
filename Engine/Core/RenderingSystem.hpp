#pragma once
#include <memory>
#include <set>
#include <vector>

#include "../Rendering/RenderCommand.hpp"
#include "Subsystem.hpp"
#include "Types.h"

class Material;
class Renderable;

class RenderingSystem : public Subsystem<RenderingSystem>
{
public:
  void RegisterMaterial(Material* material);
  void DeregisterMaterial(uint32 id);
  
  void Submit(const RenderCommand& renderCommand);
  
  ~RenderingSystem();
  
private:
  RenderingSystem();
  
private:
  std::set<RenderCommand> _renderQueue;
  std::vector<Material*> _materials;
};

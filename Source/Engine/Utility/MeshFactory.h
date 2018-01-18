#pragma once
#include <memory>

#include "../Core/Types.hpp"

namespace Rendering
{
class StaticMesh;
}

class MeshFactory
{
public:
  static std::shared_ptr<Rendering::StaticMesh> CreateCube();

  static std::shared_ptr<Rendering::StaticMesh> CreatePlane(uint32 density = 1);
};
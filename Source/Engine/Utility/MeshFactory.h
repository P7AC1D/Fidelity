#pragma once
#include <functional>
#include <memory>

#include "../Core/Types.hpp"

namespace Rendering
{
class StaticMesh;
}

typedef std::function<float32(float32 x, float32 y)> HeightFunction;

class MeshFactory
{
public:
  static std::shared_ptr<Rendering::StaticMesh> CreateCube();

  static std::shared_ptr<Rendering::StaticMesh> CreatePlane(uint32 density = 1);
};
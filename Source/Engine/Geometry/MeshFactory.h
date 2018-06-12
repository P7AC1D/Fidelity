#pragma once
#include <functional>
#include <memory>

#include "../Core/Types.hpp"

class StaticMesh;

class MeshFactory
{
public:
  static std::shared_ptr<StaticMesh> CreateCube();

  static std::shared_ptr<StaticMesh> CreatePlane(uint32 density = 1);

  static std::shared_ptr<StaticMesh> CreateIcosphere(uint32 recursionCount = 4);
};

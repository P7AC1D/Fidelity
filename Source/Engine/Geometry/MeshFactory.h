#pragma once
#include <functional>
#include <memory>

#include "../Core/Types.hpp"

class StaticMesh;

class MeshFactory
{
public:
  static std::shared_ptr<StaticMesh> createCube();

  static std::shared_ptr<StaticMesh> createPlane(uint32 density = 1);

  static std::shared_ptr<StaticMesh> createUvSphere(uint32 verticals = 10, uint32 horizontals = 10);

  static std::shared_ptr<StaticMesh> createIcosphere(uint32 recursionCount = 4);
};

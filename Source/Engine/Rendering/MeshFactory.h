#pragma once

#include <memory>

namespace Rendering
{
class StaticMesh;

class MeshFactory
{
public:
  static std::shared_ptr<StaticMesh> CreateCube();
};
}

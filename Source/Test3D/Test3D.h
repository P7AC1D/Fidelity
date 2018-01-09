#pragma once
#include <memory>

#include "../Engine/Core/Application.h"
#include "../Engine/SceneManagement/WorldObject.h"

namespace SceneManagement
{
class SceneNode;
class OrbitalCamera;
class WorldObject;
}

class Test3D : public Application
{
public:
  Test3D(const ApplicationDesc& desc);

  void OnStart() override;
  void OnUpdate(uint32 dtMs) override;

private:
  std::shared_ptr<SceneManagement::OrbitalCamera> _camera;
  std::shared_ptr<SceneManagement::WorldObject> _object;
  std::shared_ptr<SceneManagement::SceneNode> _sceneNode;
  uint32 _prevMouseX;
  uint32 _prevMouseY;
  bool _rotatingCamera;
};

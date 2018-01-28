#pragma once
#include <memory>

#include "../Engine/Core/Application.h"
#include "../Engine/SceneManagement/WorldObject.h"

class Light;
class OrbitalCamera;
class WorldObject;

class Test3D : public Application
{
public:
  Test3D(const ApplicationDesc& desc);

  void OnStart() override;
  void OnUpdate(uint32 dtMs) override;

private:
  std::shared_ptr<OrbitalCamera> _camera;
  std::shared_ptr<WorldObject> _object;
  uint32 _prevMouseX;
  uint32 _prevMouseY;
  bool _rotatingCamera;
  Light* _light;
};

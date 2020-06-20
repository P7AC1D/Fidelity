#pragma once
#include <memory>

#include "../Engine/Core/Application.h"
#include "../Engine/Maths/Vector3.hpp"

class Degree;
class Light;
class CameraNode;
class WorldObject;

class Test3D : public Application
{
public:
  Test3D(const ApplicationDesc& desc);

  void OnStart() override;
  void OnUpdate(uint32 dtMs) override;

private:
  void RotateCamera(const Degree& deltaX, const Degree& deltaY, int32 dtMs) const;
  void ZoomCamera(float32 delta, int32 dtMs) const;

  Vector3 _cameraTarget;
  std::shared_ptr<CameraNode> _camera;
  std::shared_ptr<WorldObject> _object;
};

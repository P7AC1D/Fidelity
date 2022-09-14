#pragma once
#include <memory>

#include "../Engine/Core/Fidelity.h"

class GameObject;

class Test3D : public Application
{
public:
  Test3D(const ApplicationDesc &desc);

  void OnStart() override;
  void OnUpdate(uint32 dtMs) override;

private:
  void RotateCamera(const Degree &deltaX, const Degree &deltaY, int32 dtMs);
  void ZoomCamera(float32 delta, int32 dtMs);

  Vector3 _cameraTarget;
  GameObject *_camera;
};

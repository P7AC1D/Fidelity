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
  void RotateCamera(int32 deltaX, int32 deltaY, uint32 dtMs);
  void FpsCameraLook(int32 deltaX, int32 deltaY, uint32 dtMs);
  void ZoomCamera(int32 delta, uint32 dtMs);
  void TranslateCamera(float32 forward, float32 right);

  Vector2I _lastMousePos;
  Vector3 _cameraTarget;
  GameObject *_camera;
};

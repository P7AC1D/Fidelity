#pragma once
#include <memory>

#include "../Engine/Core/Fidelity.h"

class GameObject;

class Sponza : public Application
{
public:
  Sponza(const ApplicationDesc &desc);

  void onStart() override;
  void onUpdate(uint32 dtMs) override;

private:
  void rotateCamera(int32 deltaX, int32 deltaY, uint32 dtMs);
  void fpsCameraLook(int32 deltaX, int32 deltaY, uint32 dtMs);
  void zoomCamera(int32 delta, uint32 dtMs);
  void translateCamera(float32 forward, float32 right);

  Vector2I _lastMousePos;
  Vector3 _cameraTarget;
  GameObject *_camera;
};

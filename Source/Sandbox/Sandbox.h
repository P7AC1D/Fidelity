#pragma once
#include <memory>

#include "../Engine/Core/Fidelity.h"

class Sandbox : public Application
{
public:
  Sandbox(const ApplicationDesc &desc);

  void OnStart() override;
  void OnUpdate(uint32 dtMs) override;

private:
  void RotateCamera(const Degree &deltaX, const Degree &deltaY, int32 dtMs) const;
  void ZoomCamera(float32 delta, int32 dtMs) const;

  Vector3 _cameraTarget;
};

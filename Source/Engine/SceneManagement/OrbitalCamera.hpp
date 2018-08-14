#pragma once
#include "Camera.hpp"

class OrbitalCamera : public Camera
{
public:
  OrbitalCamera(float32 rotationAcceleration = 1.0f, float32 zoomAcceleration = 1.0f);

  void RotateAboutTarget(const Degree& deltaX, const Degree& deltaY, int32 dtMs);
  void Zoom(float32 delta, int32 dtMs);

private:
  float32 _rotationAcceleration;
  float32 _zoomAcceleration;
};

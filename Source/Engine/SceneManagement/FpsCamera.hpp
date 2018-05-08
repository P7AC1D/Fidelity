#pragma once
#include "Camera.hpp"

class FpsCamera : public Camera
{
public:
  FpsCamera(float32 moveAcceleration = 1.0f, float32 lookAcceleration = 1.0f);
  ~FpsCamera();

  void StrafeRight(int32 dtMs);
  void StrafeLeft(int32 dtMs);
  void MoveForward(int32 dtMs);
  void MoveBackward(int32 dtMs);
  void Rotate(const Degree& degreeX, const Degree& degreeY, int32 dtMs);

private:
  float32 _moveAccel;
  float32 _lookAccel;
};
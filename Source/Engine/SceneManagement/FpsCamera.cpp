#include "FpsCamera.hpp"

FpsCamera::FpsCamera(float32 moveAcceleration, float32 lookAcceleration):
  _moveAccel(25.0f),
  _lookAccel(0.5f)
{
}

FpsCamera::~FpsCamera()
{
}

void FpsCamera::StrafeRight(int32 dtMs)
{
  float32 velocity(_moveAccel * dtMs * 0.001f);
  Vector3 positionDelta(GetCameraRight() * velocity);
  LookAt(positionDelta + GetPosition(), GetTarget());
}

void FpsCamera::StrafeLeft(int32 dtMs)
{
  float32 velocity(_moveAccel * dtMs * 0.001f);
  Vector3 positionDelta(-GetCameraRight() * velocity);
  LookAt(positionDelta + GetPosition(), GetTarget());
}

void FpsCamera::MoveForward(int32 dtMs)
{
  float32 velocity(_moveAccel * dtMs * 0.001f);
  Vector3 positionDelta(-GetCameraForward() * velocity);
  LookAt(positionDelta + GetPosition(), GetTarget());
}

void FpsCamera::MoveBackward(int32 dtMs)
{
  float32 velocity(_moveAccel * dtMs * 0.001f);
  Vector3 positionDelta(GetCameraForward() * velocity);
  LookAt(positionDelta + GetPosition(), GetTarget());
}

void FpsCamera::Rotate(const Degree& deltaX, const Degree& deltaY, int32 dtMs)
{
  float32 velocity(_lookAccel * dtMs * 0.001f);
  Quaternion pitch(GetCameraRight(), deltaX.InRadians() * velocity);
  Quaternion yaw(GetCameraUp(), deltaY.InRadians() * velocity);
  Quaternion rotation(pitch * yaw);
  Vector3 forward(GetTarget() - GetPosition());
  Vector3 newForward(rotation.Rotate(forward));
  LookAt(GetPosition(), newForward);
}

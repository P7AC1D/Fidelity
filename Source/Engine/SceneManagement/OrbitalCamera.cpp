#include "OrbitalCamera.hpp"

OrbitalCamera::OrbitalCamera(float32 rotationAcceleration, float32 zoomAcceleration) :
  _rotationAcceleration(rotationAcceleration),
  _zoomAcceleration(zoomAcceleration)
{
}

void OrbitalCamera::RotateAboutTarget(const Degree& deltaX, const Degree& deltaY, int32 dtMs)
{
  float32 velocity(_rotationAcceleration * static_cast<float32>(dtMs) * 0.001f);
  Quaternion pitch(GetCameraRight(), velocity * deltaX.InRadians());
  Quaternion yaw(GetCameraUp(), velocity * deltaY.InRadians());
  Quaternion rotation(pitch * yaw);
  Vector3 newPosition(rotation.Rotate(GetPosition()));
  LookAt(newPosition, GetTarget());
}

void OrbitalCamera::Zoom(float32 delta, int32 dtMs)
{
  float32 distanceToTarget = Vector3::Length(GetPosition() - GetTarget());
  float32 velocity(_zoomAcceleration * distanceToTarget * static_cast<float32>(dtMs) * 0.001f);
  Vector3 cameraTarget(GetTarget());
  Vector3 oldPosition(GetPosition());
  Vector3 newPosition(oldPosition + GetCameraForward() * velocity * -delta);

  Vector3 cameraToOldPos(oldPosition - cameraTarget);
  Vector3 cameraToNewPos(newPosition - cameraTarget);
  if (Vector3::Dot(cameraToNewPos, cameraToOldPos) < 0.0f)
  {
    newPosition = cameraTarget + GetCameraForward() * 0.1f;
  }
  LookAt(newPosition, GetTarget());
}

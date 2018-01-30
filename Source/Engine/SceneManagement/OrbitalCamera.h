#pragma once

#include "../Core/Types.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Maths/Vector3.hpp"
#include "../Maths/Vector4.hpp"

class OrbitalCamera
{
public:
  OrbitalCamera();
  OrbitalCamera(float32 phi, float32 theta, float32 radius);
  void Rotate(float32 dPhi, float32 dTheta);
  void Zoom(float32 dr);
  void Pan(float32 dx, float32 dy);
  void UpdateProjMat(int32 clientWidth, int32 clientHeight, float32 nearClip, float32 farClip);

  void SetPosition(const Vector3& position);

  Vector3 GetPos() const;
  Vector3 GetDirection() const;
  Matrix4 GetViewMat();
  Matrix4 GetProjMat() const;

private:
  void UpdateViewMatrix();
  Vector3 PositionToCartesian() const;

  float32 m_phi;
  float32 m_theta;
  float32 m_radius;

  Vector3 m_target;
  Matrix4 m_view;
  Matrix4 m_proj;

  bool m_needsUpdate;
};
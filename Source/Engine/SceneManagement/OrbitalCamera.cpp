#include "OrbitalCamera.h"

#include "../Maths/Degree.hpp"
#include "../Maths/Math.hpp"
#include "../Maths/Radian.hpp"

OrbitalCamera::OrbitalCamera()
: m_phi(0.0f),
  m_theta(0.0f),
  m_radius(1.0f),
  m_target(Vector3::Zero),
  m_view(Matrix4::Identity),
  m_proj(Matrix4::Identity),
  m_needsUpdate(true)
{
}

OrbitalCamera::OrbitalCamera(float32 phi, float32 theta, float32 radius)
: m_phi(phi),
  m_theta(theta),
  m_radius(radius),
  m_target(Vector3::Zero),
  m_view(Matrix4::Identity),
  m_proj(Matrix4::Identity),
  m_needsUpdate(true)
{
}

void OrbitalCamera::Rotate(float32 dPhi, float32 dTheta)
{
  m_needsUpdate = true;
  m_phi += dPhi;
  m_theta += dTheta;
  m_theta = Math::Clamp(m_theta, 1.0f, 180.0f);

  // Keep Phi within -360 -> 360 degrees.
  if (m_phi > 360.0f)
  {
    m_phi -= 360.0f;
  }
  else if (m_phi < -360.0f)
  {
    m_phi += 360.0f;
  }
}

void OrbitalCamera::Zoom(float dr)
{
  m_needsUpdate = true;
  m_radius -= dr;
  m_radius = Math::Clamp(m_radius, 1.0f, 100.0f);
}

void OrbitalCamera::Pan(float dx, float dy)
{
  m_needsUpdate = true;
  Vector3 worldUp(0.0f, 1.0f, 0.0f);

  Vector3 forward = Vector3::Normalize(PositionToCartesian());
  Vector3 right = worldUp == forward ? Vector3::Normalize(Vector3::Cross(Vector3(1.0f, 0.0f, 0.0f), forward)) : Vector3::Normalize(Vector3::Cross(worldUp, forward));
  Vector3 up = Vector3::Normalize(Vector3::Cross(forward, right));

  m_target += (right * dx) + (up * dy);
}

void OrbitalCamera::UpdateProjMat(int32 clientWidth, int32 clientHeight, float32 nearClip, float32 farClip)
{
  //m_proj = Matrix4::Perspective(Degree(67.67f), clientWidth / static_cast<float32>(clientHeight), nearClip, farClip);
  m_proj = Matrix4::Orthographic(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);
}

void OrbitalCamera::SetPosition(const Vector3& position)
{
  Vector3 pos = position - m_target;
  m_radius = std::sqrtf(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
  m_theta = Math::ACos(pos[1] / m_radius).InDegrees();
  m_phi = Math::ATan(pos[1] / pos[2]).InDegrees();
}

Vector3 OrbitalCamera::GetPos() const
{
  return m_target + PositionToCartesian();
}

Matrix4 OrbitalCamera::GetViewMat()
{
  if (m_needsUpdate)
  {
    UpdateViewMatrix();
    m_needsUpdate = false;
  }
  return m_view;
}

Matrix4 OrbitalCamera::GetProjMat() const
{
  return m_proj;
}

void OrbitalCamera::UpdateViewMatrix()
{
  Vector3 eye = PositionToCartesian();

  Vector3 forward = Vector3::Normalize(eye);
  Vector3 worldUp = Vector3(0.0f, 1.0f, 0.0f);
  Vector3 right = worldUp == forward ? Vector3::Normalize(Vector3::Cross(Vector3(0.0f, 0.0f, -1.0f), forward)) : Vector3::Normalize(Vector3::Cross(worldUp, forward));
  Vector3 up = Vector3::Normalize(Vector3::Cross(forward, right));
  
  Vector3 cen = Vector3::Normalize(m_target - PositionToCartesian());
 // m_view = Matrix4::LookAt(eye, cen, up);
  
  m_view = Matrix4::LookAt(Vector3::Identity, Vector3::Zero, Vector3(0.0f, 1.0f, 0.0f));
}

Vector3 OrbitalCamera::PositionToCartesian() const
{
  float32 phir = Math::Deg2Rad * m_phi;
  float32 thetar = Math::Deg2Rad * m_theta;
  float32 x = m_radius * std::sinf(phir) * std::sinf(thetar);
  float32 y = m_radius * std::cosf(thetar);
  float32 z = m_radius * std::cosf(phir) * std::sinf(thetar);
  return Vector3(x, y, z);
}
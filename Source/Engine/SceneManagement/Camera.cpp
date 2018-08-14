#include "Camera.hpp"

#include <cmath>
#include "../Maths/Matrix3.hpp"
#include "../Maths/Ray.hpp"

Camera::Camera() :
  _width(0),
  _height(0),
  _fov(0.0f),
  _near(0.0f),
  _far(0.0f),
  _view(Matrix4::Identity),
  _proj(Matrix4::Identity)
{
}

void Camera::LookAt(const Vector3& position, const Vector3& target, const Vector3& up)
{
  Vector3 camDir = Vector3::Normalize(target - position);
  if (std::abs(Vector3::Dot(camDir, up)))
  {
    camDir += 0.00001f;
  }
  Vector3 camRight = Vector3::Normalize(Vector3::Cross(camDir, up));
  Vector3 camUp = Vector3::Normalize(Vector3::Cross(camRight, camDir));

  Matrix3 view(Matrix3::Identity);
  view[0][0] = camRight.X;
  view[1][0] = camRight.Y;
  view[2][0] = camRight.Z;
  view[0][1] = camUp.X;
  view[1][1] = camUp.Y;
  view[2][1] = camUp.Z;
  view[0][2] = -camDir.X;
  view[1][2] = -camDir.Y;
  view[2][2] = -camDir.Z;

  _position = position;
  _target = target;

  _orientation = view;
  UpdateView();
}

void Camera::SetPerspective(const Degree& fovy, int32 width, int32 height, float32 nearClip, float32 farClip)
{
  _width = width;
  _height = height;
  _fov = fovy;
  _near = nearClip;
  _far = farClip;
  UpdateProjection();
}

Ray Camera::ScreenPointToRay(const Vector2I& screenPoint) const
{
  // Screen space to NDC
  float32 x = (2.0f * screenPoint.X) / _width - 1.0f;
  float32 y = 1.0f - (2.0f * screenPoint.Y) / _height;
  
  // NDC to 4D Homogenous Coords
  Vector4 rayDir(x, y -1.0f, 1.0f);
  
  // 4D Coords to View Space
  rayDir = _projInvs * rayDir;
  
  // View Space to World Space
  rayDir = Vector4(rayDir.X, rayDir.Y, -1.0f, 0.0f);
  rayDir = _viewInvs * rayDir;
  
  return Ray(_position, Vector3::Normalize(Vector3(rayDir.X, rayDir.Y, rayDir.Z)));
}

const Matrix4& Camera::GetView()
{
  UpdateView();
  return _view;
}

const Matrix4& Camera::GetProjection()
{
  UpdateProjection();
  return _proj;
}

void Camera::SetPosition(const Vector3& position)
{
  if (position != _position)
  {
    _position = position;
    UpdateView();
  }
}

void Camera::SetOrientation(const Quaternion& orientation)
{
  if (orientation != _orientation)
  {
    _orientation = Quaternion::Normalize(orientation);
    UpdateView();
  }
}

void Camera::UpdateView()
{
  Matrix4 rotation(_orientation);
  Matrix4 translation(Matrix4::Translation(_position));
  translation[3][0] = -translation[3][0];
  translation[3][1] = -translation[3][1];
  translation[3][2] = -translation[3][2];

  _view = rotation * translation;
  _viewInvs = _view.Inverse();

  _right = Vector3(_view[0][0], _view[1][0], _view[2][0]);
  _up = Vector3(_view[0][1], _view[1][1], _view[2][1]);
  _forward = Vector3(_view[0][2], _view[1][2], _view[2][2]);
}

void Camera::UpdateProjection()
{
  _proj = Matrix4::Perspective(_fov, _width / static_cast<float32>(_height), _near, _far);
  _projInvs = _proj.Inverse();
	UpdateFrustrum();
}

void Camera::UpdateFrustrum()
{
	_frustrum = Frustrum(_proj);
}

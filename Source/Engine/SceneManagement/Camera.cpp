#include "Camera.hpp"

#include "../Maths/Matrix3.hpp"
#include "../Rendering/ConstantBuffer.h"

using namespace Rendering;

Camera::Camera() :
  _width(0),
  _height(0),
  _fov(0.0f),
  _near(0.0f),
  _far(0.0f),
  _view(Matrix4::Identity),
  _proj(Matrix4::Identity)
{
  InitializeBuffer();
}

Camera::~Camera()
{
}

void Camera::LookAt(const Vector3& position, const Vector3& target, const Vector3& up)
{
  Vector3 camDir = Vector3::Normalize(target - position);
  if (abs(Vector3::Dot(camDir, up)))
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

const uint32 Camera::GetInternalBufferIndex() const
{
  return _buffer->GetId();
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

void Camera::InitializeBuffer()
{
  _buffer.reset(new ConstantBuffer(144));
}

void Camera::UpdateView()
{
  Matrix4 rotation(_orientation);
  Matrix4 translation(Matrix4::Translation(_position));
  translation[3][0] = -translation[3][0];
  translation[3][1] = -translation[3][1];
  translation[3][2] = -translation[3][2];

  _view = rotation * translation;

  _right = Vector3(_view[0][0], _view[1][0], _view[2][0]);
  _up = Vector3(_view[0][1], _view[1][1], _view[2][1]);
  _forward = Vector3(_view[0][2], _view[1][2], _view[2][2]);

  UpdateBuffer();
}
void Camera::UpdateProjection()
{
  _proj = Matrix4::Perspective(_fov, _width / static_cast<float32>(_height), _near, _far);
  UpdateBuffer();
}

void Camera::UpdateBuffer()
{
  _buffer->UploadData(0, 64, &_proj[0]);
  _buffer->UploadData(64, 64, &_view[0]);
  _buffer->UploadData(128, 12, &_position[0]);
}
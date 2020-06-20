#include "Transform.h"

Transform::Transform() : 
  _scale(Vector3::Identity),
  _position(Vector3::Zero),
  _rotation(Quaternion::Identity),
  _transform(Matrix4::Identity),
  _modified(true)
{
}

Transform::Transform(const Matrix4& mat) :
  _scale(Vector3::Identity),
  _position(Vector3::Zero),
  _rotation(Quaternion::Identity),
  _transform(mat),
  _modified(true)
{
}

Transform* Transform::Translate(const Vector3& translation)
{
  _position += translation;
  _modified = true;
  return this;
}

Transform* Transform::Rotate(const Quaternion& rotation)
{
  _rotation = rotation * _rotation;
  _modified = true;
  return this;
}

Transform* Transform::Scale(const Vector3& scale)
{
  _scale += scale;
  _modified = true;
  return this;
}

Transform* Transform::SetRotation(const Quaternion& rotation)
{
  _rotation = rotation;
  _modified = true;
  return this;
}

Transform* Transform::SetPosition(const Vector3& position)
{
  _position = position;
  _modified = true;
  return this;
}

Transform* Transform::SetScale(const Vector3& scale)
{
  _scale = Vector3(std::max(scale.X, 0.0f), std::max(scale.Y, 0.0f), std::max(scale.Z, 0.0f));
  _modified = true;
  return this;
}

Transform* Transform::LookAt(const Vector3& eye, const Vector3& target, const Vector3& up)
{
  Vector3 camDir = Vector3::Normalize(target - eye);
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

  _position = eye;
  _rotation = view;
  _modified = true;
  return this;
}

Quaternion Transform::GetRotation() const
{
  return _rotation;
}

Vector3 Transform::GetPosition() const
{
  return _position;
}

Vector3 Transform::GetScale() const
{
  return _scale;
}

Matrix4 Transform::GetMatrix() const
{
  return _transform;
}

Vector3 Transform::GetForward() const
{
  return Vector3(_transform[0][2], _transform[1][2], _transform[2][2]);
}

Vector3 Transform::GetUp() const
{
	return Vector3(_transform[0][1], _transform[1][1], _transform[2][1]);
}

Vector3 Transform::GetRight() const
{
  return Vector3(_transform[0][0], _transform[1][0], _transform[2][0]);
}

void Transform::Update()
{
  UpdateTransform();
}

void Transform::UpdateTransform()
{
  Matrix4 translation = Matrix4::Translation(_position);
  Matrix4 scale = Matrix4::Scaling(_scale);
  Matrix4 rotation = Matrix4::Rotation(_rotation);
  _transform = translation * scale * rotation;
}

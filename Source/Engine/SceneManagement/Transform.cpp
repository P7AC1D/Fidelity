#include "Transform.h"

Transform::Transform() : 
  _scale(Vector3::Identity),
  _position(Vector3::Zero),
  _rotation(Quaternion::Identity),
  _transform(Matrix4::Identity),
  _modified(true)
{
}

Transform* Transform::Translate(const Vector3& translation)
{
  _modified = true;
  _position += translation;
  return this;
}

Transform* Transform::Rotate(const Quaternion& rotation)
{
  _modified = true;
  _rotation = rotation * _rotation;
  return this;
}

Transform* Transform::Scale(const Vector3& scale)
{
  _modified = true;
  _scale += scale;
  return this;
}

Transform* Transform::SetRotation(const Quaternion& rotation)
{
  _modified = true;
  _rotation = rotation;
  return this;
}

Transform* Transform::SetPosition(const Vector3& position)
{
  _modified = true;
  _position = position;
  return this;
}

Transform* Transform::SetScale(const Vector3& scale)
{
  _modified = true;
  _scale = Vector3(std::max(scale.X, 0.0f), std::max(scale.Y, 0.0f), std::max(scale.Z, 0.0f));
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

Matrix4 Transform::Get()
{
  if (_modified)
  {
    UpdateTransform();
  }
  return _transform;
}

void Transform::UpdateTransform()
{
  Matrix4 translation = Matrix4::Translation(_position);
  Matrix4 scale = Matrix4::Scaling(_scale);
  Matrix4 rotation = Matrix4::Rotation(_rotation);
  _transform = translation * scale * rotation;
  _modified = false;
}

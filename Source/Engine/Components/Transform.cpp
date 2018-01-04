#include "Transform.h"

namespace Components
{
Transform::Transform() : 
  _scale(Vector3::Identity),
  _position(Vector3::Zero),
  _rotation(Quaternion::Zero),
  _transform(Matrix4::Identity),
  _transformNeedsUpdate(true),
  Component("Transform")
{
}

Transform* Transform::Translate(const Vector3& translation)
{
  _transformNeedsUpdate = true;
  _position += translation;
  return this;
}

Transform* Transform::Rotate(const Quaternion& rotation)
{
  _transformNeedsUpdate = true;
  _rotation = rotation * _rotation;
  return this;
}

Transform* Transform::Scale(const Vector3& scale)
{
  _transformNeedsUpdate = true;
  _scale += scale;
  return this;
}

Transform* Transform::SetRotation(const Quaternion& rotation)
{
  _transformNeedsUpdate = true;
  _rotation = rotation;
  return this;
}

Transform* Transform::SetPosition(const Vector3& position)
{
  _transformNeedsUpdate = true;
  _position = position;
  return this;
}

Transform* Transform::SetScale(const Vector3& scale)
{
  _transformNeedsUpdate = true;
  _scale = scale;
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
  if (_transformNeedsUpdate)
  {
    UpdateTransform();
  }
  return _transform;
}

void Transform::UpdateTransform()
{
  _transform = Matrix4::Translation(_position) * Matrix4::Scaling(_scale) * Matrix4::Rotation(_rotation);
  _transformNeedsUpdate = false;
}
}
#include "Transform.h"

#include <limits>

#include "../Maths/Math.hpp"

Transform::Transform() : _scale(Vector3::Identity),
                         _position(Vector3::Zero),
                         _rotation(Quaternion::Identity),
                         _transform(Matrix4::Identity),
                         _modified(true)
{
}

Transform::Transform(const Matrix4 &mat) : _transform(mat),
                                           _modified(true)
{
  Vector3 translation;
  Vector3 scale;
  Matrix3 rotation;
  mat.Decompose(translation, scale, rotation);
  _position = translation;
  _scale = scale;
  _rotation = rotation;
}

void Transform::update(float32 dt)
{
  if (_modified)
  {
    updateTransform();
    _modified = false;
  }
}

Transform &Transform::translate(const Vector3 &translation)
{
  _position += translation;
  _modified = true;
  return *this;
}

Transform &Transform::rotate(const Quaternion &rotation)
{
  _rotation = rotation * _rotation;
  _modified = true;
  return *this;
}

Transform &Transform::scale(const Vector3 &scale)
{
  _scale += scale;
  _scale = Math::Clamp(_scale, 0.01f, std::numeric_limits<float32>::max());

  _modified = true;
  return *this;
}

Transform &Transform::setRotation(const Quaternion &rotation)
{
  _rotation = rotation;
  _modified = true;
  return *this;
}

Transform &Transform::setPosition(const Vector3 &position)
{
  _position = position;
  _modified = true;
  return *this;
}

Transform &Transform::setScale(const Vector3 &scale)
{
  _scale = Vector3(std::max(scale.X, 0.0f), std::max(scale.Y, 0.0f), std::max(scale.Z, 0.0f));
  _modified = true;
  return *this;
}

Transform &Transform::lookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up)
{
  Matrix3 view(Matrix3::LookAt(eye, target, up));
  _position = eye;
  _rotation = view;
  _modified = true;
  return *this;
}

void Transform::updateTransform()
{
  Matrix4 translation = Matrix4::Translation(_position);
  Matrix4 scale = Matrix4::Scaling(_scale);
  Matrix4 rotation = Matrix4::Rotation(_rotation);
  _transform = translation * scale * rotation;
}

Transform Transform::operator*(const Transform &rhs) const
{
  Transform result;
  Vector3 position = _position + rhs._position * _scale;
  Vector3 scale = _scale * rhs._scale;
  Quaternion orientation = _rotation * rhs._rotation;
  position = orientation.Rotate(position);

  result.setPosition(position);
  result.setScale(scale);
  result.setRotation(orientation);

  return result;
}
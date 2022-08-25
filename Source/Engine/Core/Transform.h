#pragma once

#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"

class Transform
{
public:
  Transform();
  explicit Transform(const Matrix4 &mat);

  Transform &translate(const Vector3 &translation);
  Transform &rotate(const Quaternion &rotation);
  Transform &scale(const Vector3 &scale);

  Transform &setRotation(const Quaternion &rotation);
  Transform &setPosition(const Vector3 &position);
  Transform &setScale(const Vector3 &scale);
  Transform &lookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up = Vector3(0.0f, 1.0f, 0.0f));

  Quaternion getRotation();
  Vector3 getPosition();
  Vector3 getScale();
  Matrix4 getMatrix();

  Vector3 getForward();
  Vector3 getUp();
  Vector3 getRight();

private:
  void updateTransform();

  Vector3 _scale;
  Vector3 _position;
  Quaternion _rotation;
  Matrix4 _transform;

  bool _modified;
};
#pragma once

#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"

class Transform
{
public:
  Transform();
  explicit Transform(const Matrix4 &mat);

  void update(float32 dt);

  Transform &translate(const Vector3 &translation);
  Transform &rotate(const Quaternion &rotation);
  Transform &scale(const Vector3 &scale);

  Transform &setRotation(const Quaternion &rotation);
  Transform &setPosition(const Vector3 &position);
  Transform &setScale(const Vector3 &scale);
  Transform &lookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up = Vector3(0.0f, 1.0f, 0.0f));

  Quaternion getRotation() const { return _rotation; }
  Vector3 getPosition() const { return _position; }
  Vector3 getScale() const { return _scale; }
  Matrix4 getMatrix() const { return _transform; }

  Vector3 getForward() const { return Vector3(_transform[0][2], _transform[1][2], _transform[2][2]); }
  Vector3 getUp() const { return Vector3(_transform[0][1], _transform[1][1], _transform[2][1]); }
  Vector3 getRight() const { return Vector3(_transform[0][0], _transform[1][0], _transform[2][0]); }

  bool modified() const { return _modified; }

private:
  void updateTransform();

  Vector3 _scale;
  Vector3 _position;
  Quaternion _rotation;
  Matrix4 _transform;

  bool _modified;
};
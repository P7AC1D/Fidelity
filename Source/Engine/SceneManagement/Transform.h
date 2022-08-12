#pragma once

#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"

class Transform
{
public:
  Transform();
  explicit Transform(const Matrix4& mat);

  Transform& Translate(const Vector3& translation);
  Transform& Rotate(const Quaternion& rotation);
  Transform& Scale(const Vector3& scale);

  Transform& SetRotation(const Quaternion& rotation);
  Transform& SetPosition(const Vector3& position);
  Transform& SetScale(const Vector3& scale);
  Transform& LookAt(const Vector3& eye, const Vector3& target, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f));

  Quaternion GetRotation() const;
  Vector3 GetPosition() const;
  Vector3 GetScale() const;
  Matrix4 GetMatrix() const;

  Vector3 GetForward() const;
  Vector3 GetUp() const;
  Vector3 GetRight() const;

  void Update();

	bool Modified() const { return _modified; }

private:
  void UpdateTransform();

  Vector3 _scale;
  Vector3 _position;
  Quaternion _rotation;
  Matrix4 _transform;

  bool _modified;
};
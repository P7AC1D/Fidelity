#pragma once

#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"

class Transform
{
public:
  Transform();

  Transform* Translate(const Vector3& translation);
  Transform* Rotate(const Quaternion& rotation);
  Transform* Scale(const Vector3& scale);

  Transform* SetRotation(const Quaternion& rotation);
  Transform* SetPosition(const Vector3& position);
  Transform* SetScale(const Vector3& scale);

  Quaternion GetRotation() const;
  Vector3 GetPosition() const;
  Vector3 GetScale() const;
  Matrix4 Get();

	bool Modified() const { return _transformNeedsUpdate; }

private:
  void UpdateTransform();

  Vector3 _scale;
  Vector3 _position;
  Quaternion _rotation;
  Matrix4 _transform;

  bool _transformNeedsUpdate;
};
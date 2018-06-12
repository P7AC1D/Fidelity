#pragma once
#include <memory>

#include "../Maths/Degree.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Radian.hpp"
#include "../Maths/Vector3.hpp"

class Camera
{
public:
  Camera();

  void LookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f));
  void SetPerspective(const Degree& fovy, int32 width, int32 height, float32 nearClip, float32 farClip);

  const Matrix4& GetView();
  const Matrix4& GetProjection();
  inline const Vector3& GetCameraRight() const { return _right; }
  inline const Vector3& GetCameraUp() const { return _up; }
  inline const Vector3& GetCameraForward() const { return _forward; }
  inline const Vector3& GetPosition() const { return _position; }
  inline const Vector3& GetTarget() const { return _target; }
  inline const Quaternion& GetOrientation() const { return _orientation; }

private:
  void SetPosition(const Vector3& position);
  void SetOrientation(const Quaternion& orientation);
  void UpdateView();
  void UpdateProjection();

private:
  int32 _width;
  int32 _height;
  Radian _fov;
  float32 _near;
  float32 _far;
  Matrix4 _view;
  Matrix4 _proj;
  Vector3 _position;
  Vector3 _target;
  Vector3 _up;
  Vector3 _right;
  Vector3 _forward;
  Quaternion _orientation;
};

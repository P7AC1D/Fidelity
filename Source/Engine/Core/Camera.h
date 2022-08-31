#pragma once

#include "GameObject.h"
#include "Types.hpp"
#include "Maths.h"

class Drawable;

class Camera : public Transform
{
public:
  Camera();

  void drawInspector();

  void update(float64 dt);

  void lookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up = Vector3(0.0f, 1.0f, 0.0f));

  void setPerspective(const Degree &fovY, int32 width, int32 height, float32 nearClip, float32 farClip);
  void setHeight(int32 height);
  void setWidth(int32 width);
  void setFov(const Degree &fov);
  void setNear(float32 near);
  void setFar(float32 far);

  Matrix4 getView() const { return _view; }
  Matrix4 getProj() const { return _proj; }
  int32 getWidth() const { return _width; }
  int32 getHeight() const { return _height; }
  Radian getFov() const { return _fov; }
  float32 getNear() const { return _near; }
  float32 getFar() const { return _far; }

  bool intersectsFrustrum(const Drawable &drawable) const;
  uint32 distanceFrom(const Drawable &drawable) const;

private:
  void updateView();
  void updateProjection();

  bool _modified;

  int32 _width;
  int32 _height;
  Radian _fov;
  float32 _near;
  float32 _far;

  Matrix4 _view;
  Matrix4 _proj;

  Vector3 _position;
  Quaternion _rotation;

  Frustrum _frustrum;
};
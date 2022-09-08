#pragma once

#include "../Core/GameObject.h"
#include "../Core/Types.hpp"
#include "../Core/Maths.h"
#include "../Core/Transform.h"

class Drawable;

class Camera
{
public:
  Camera();

  void drawInspector();

  void update(float64 dt);

  Camera &setPerspective(const Degree &fovY, int32 width, int32 height, float32 nearClip, float32 farClip);
  Camera &setHeight(int32 height);
  Camera &setWidth(int32 width);
  Camera &setFov(const Degree &fov);
  Camera &setNear(float32 near);
  Camera &setFar(float32 far);

  Matrix4 getView() const { return _view; }
  Matrix4 getProj() const { return _proj; }
  int32 getWidth() const { return _width; }
  int32 getHeight() const { return _height; }
  Radian getFov() const { return _fov; }
  float32 getNear() const { return _near; }
  float32 getFar() const { return _far; }

  Transform &getTransform() { return _transform; }
  Transform getTransformCopy() const { return _transform; }
  const Frustrum &getFustrum() const { return _frustrum; }

  bool intersectsFrustrum(const Aabb &aabb) const;
  float32 distanceFrom(const Vector3 &position) const;

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

  Frustrum _frustrum;
  Transform _transform;
};
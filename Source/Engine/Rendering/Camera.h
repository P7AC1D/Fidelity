#pragma once

#include "../Core/Component.h"
#include "../Core/GameObject.h"
#include "../Core/Maths.h"
#include "../Core/Transform.h"
#include "../Core/Types.hpp"

class Drawable;

class Camera : public Component
{
public:
  Camera();

  void drawInspector() override;

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
  float32 getAspectRatio() const { return _width / static_cast<float32>(_height); }
  const Transform &getParentTransform() const { return _transform; }

  const Frustrum &getFustrum() const { return _frustrum; }

  bool contains(const Aabb &aabb) const;
  float32 distanceFrom(const Vector3 &position) const;

private:
  void onUpdate(float32 dt) override;
  void onNotify(const GameObject &gameObject) override;

  void updateView(const Transform &transform);
  void updateProjection();

  bool _modified;

  int32 _width;
  int32 _height;
  Radian _fov;
  float32 _near;
  float32 _far;

  Matrix4 _view;
  Matrix4 _proj;

  Transform _transform;

  Frustrum _frustrum;
};
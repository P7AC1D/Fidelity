#pragma once
#include <array>
#include <string>
#include <memory>

#include "../Core/Component.h"
#include "../Core/GameObject.h"
#include "../Core/Maths.h"
#include "../Maths/Radian.hpp"

class StaticMesh;
class Material;

class Drawable : public Component
{
public:
  Drawable();

  void drawInspector() override;

  void setMesh(std::shared_ptr<StaticMesh> mesh);
  void setMaterial(std::shared_ptr<Material> material);

  std::shared_ptr<StaticMesh> getMesh() const { return _mesh; }
  std::shared_ptr<Material> getMaterial() const { return _material; }

  void enableDrawAabb(bool enable) { _drawAabb = enable; }

  const Aabb &getAabb() const { return _currAabb; }
  bool shouldDrawAabb() const { return _drawAabb; }

  Vector3 getPosition() const { return _position; }
  Matrix4 getMatrix() const { return _transform; }

private:
  void onUpdate(float32 dt) override;
  void onNotify(const GameObject &gameObject) override;

  void updateAabb(Vector3 scalingDelta, Quaternion rotationDelta);

  std::shared_ptr<StaticMesh> _mesh;
  std::shared_ptr<Material> _material;

  Aabb _initAabb, _currAabb;
  std::array<Radian, 3> _currentRotationEuler;
  Vector3 _currentScale;

  Vector3 _scale;
  Vector3 _position;
  Quaternion _rotation;

  Matrix4 _transform;

  bool _drawAabb;
  bool _modified;
};
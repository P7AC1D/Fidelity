#pragma once
#include <memory>

#include "../Core/GameObject.h"
#include "../Core/Maths.h"

class StaticMesh;
class Material;

class Drawable : public GameObject
{
public:
  Drawable();

  void update(float32 dt) override;
  void drawInspector() override {}

  Drawable &setMesh(std::shared_ptr<StaticMesh> mesh);
  Drawable &setMaterial(std::shared_ptr<Material> material);

  std::shared_ptr<StaticMesh> getMesh() const { return _mesh; }
  std::shared_ptr<Material> getMaterial() const { return _material; }

  void enableDrawAabb(bool enable) { _drawAabb = enable; }

  Aabb getAabb() const { return _currAabb; }
  bool shouldDrawAabb() const { return _drawAabb; }

private:
  void updateAabb(Vector3 scalingDelta, Quaternion rotationDelta);

  std::shared_ptr<StaticMesh> _mesh;
  std::shared_ptr<Material> _material;

  Aabb _initAabb, _currAabb;
  Vector3 _currentRotationEuler;
  Vector3 _currentScale;

  bool _drawAabb;
};
#pragma once
#include <memory>
#include "GameObject.h"

class StaticMesh;
class Material;

class Drawable : public GameObject
{
public:
  void update(float32 dt) override;
  void drawInspector() override;

  Drawable &setMesh(std::shared_ptr<StaticMesh> mesh);
  Drawable &setMaterial(std::shared_ptr<Material> material);

  std::shared_ptr<StaticMesh> getMesh() const { return _mesh; }
  std::shared_ptr<StaticMesh> getMaterial() const { return _material; }

private:
  std::shared_ptr<StaticMesh>
      _mesh;
  std::shared_ptr<Material> _material;
};
#pragma once

#include <memory>
#include <string>

#include "../Core/ComponentBase.h"
#include "../Core/ComponentTypeId.h"
#include "../Core/Maths.h"
#include "../Core/Types.hpp"

class StaticMesh;
class Material;
class TransformComponent;

/// Modern Drawable component using ComponentBase.
/// Handles rendering of meshes with materials and proper transform integration.
/// Uses direct transform queries for efficiency.
class DrawableComponent : public ComponentBase
{
public:
  DrawableComponent();
  DrawableComponent(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material);
  ~DrawableComponent();

  // ComponentBase interface
  void onInitialize() override;
  void onActivate() override;
  void onDeactivate() override;
  void onUpdate(float32 dt) override;
  ComponentTypeId getTypeId() const override;
  void drawInspector() override;

  // Static type information
  static ComponentTypeId GetTypeId() { return getComponentTypeId<DrawableComponent>(); }

  // Mesh and Material management
  void setMesh(std::shared_ptr<StaticMesh> mesh);
  void setMaterial(std::shared_ptr<Material> material);

  std::shared_ptr<StaticMesh> getMesh() const { return _mesh; }
  std::shared_ptr<Material> getMaterial() const { return _material; }

  // Rendering properties
  void setVisible(bool visible) { _visible = visible; }
  bool isVisible() const { return _visible; }

  void setCastShadows(bool castShadows) { _castShadows = castShadows; }
  bool getCastShadows() const { return _castShadows; }

  void setReceiveShadows(bool receiveShadows) { _receiveShadows = receiveShadows; }
  bool getReceiveShadows() const { return _receiveShadows; }

  // AABB and bounds
  const Aabb &getWorldBounds() const;
  const Aabb &getLocalBounds() const { return _localBounds; }

  // Debug rendering
  void enableDrawAabb(bool enable) { _drawAabb = enable; }
  bool shouldDrawAabb() const { return _drawAabb; }

  // Transform integration (using direct queries)
  const Matrix4 &getWorldMatrix() const;

  // Frustum culling helpers
  const Aabb &getAabb() const { return getWorldBounds(); }

  // Change tracking for rendering optimization
  bool hasChanged() const { return _boundsValid == false; }
  void markDirty() { _boundsValid = false; }

  // Public access to transform data for external systems (rendering, culling, etc.)
  const TransformComponent* getTransform() const;
  Vector3 getWorldPosition() const;

private:
  // Core rendering data
  std::shared_ptr<StaticMesh> _mesh;
  std::shared_ptr<Material> _material;

  // Rendering flags
  bool _visible = true;
  bool _castShadows = true;
  bool _receiveShadows = true;
  bool _drawAabb = false;

  // Bounds caching
  Aabb _localBounds;
  mutable Aabb _worldBounds;
  mutable bool _boundsValid = false;

  // Helper methods
  void updateWorldBounds() const;
  void onMeshChanged();
  void drawMaterialInspector();
  void drawRenderingFlagsInspector();
  void drawTexturePreview(const std::string &textureName, int textureIndex);
};

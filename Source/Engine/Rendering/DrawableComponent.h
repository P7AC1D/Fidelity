#pragma once

#include <memory>
#include <string>

#include "../Core/IComponent.h"
#include "../Core/ComponentTypeId.h"
#include "../Core/Maths.h"
#include "../Core/Types.hpp"

class StaticMesh;
class Material;
class TransformComponent;

/// Modern Drawable component that implements IComponent interface.
/// Handles rendering of meshes with materials and proper transform integration.
class DrawableComponent : public IComponent
{
public:
  DrawableComponent();
  DrawableComponent(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material);

  // IComponent interface
  void initialize() override;
  void activate() override;
  void deactivate() override;
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

  // Rendering data access
  const Matrix4 &getWorldMatrix() const;
  Vector3 getWorldPosition() const;

  // Frustum culling helpers
  const Aabb &getAabb() const { return getWorldBounds(); }
  const TransformComponent &getCachedTransform() const;

  // Change tracking for rendering optimization
  bool hasChanged() const { return _boundsValid == false; }
  void markDirty() { _boundsValid = false; }

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

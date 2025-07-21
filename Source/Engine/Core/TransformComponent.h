#pragma once

#include <functional>
#include <vector>
#include "ComponentBase.h"
#include "ComponentTypeId.h"
#include "../Core/Maths.h"
#include "../Maths/Quaternion.hpp"
#include "Types.hpp"

/// Modern Transform component that implements ComponentBase.
/// This will eventually replace the old Transform class for new GameObjects.
class TransformComponent : public ComponentBase
{
public:
  TransformComponent();
  TransformComponent(const Vector3 &position, const Quaternion &rotation = Quaternion::Identity, const Vector3 &scale = Vector3::Identity);

  // IComponent interface
  ComponentTypeId getTypeId() const override;
  void drawInspector() override;

  // Static type information
  static ComponentTypeId GetTypeId() { return getComponentTypeId<TransformComponent>(); }

  // Transform operations
  void setPosition(const Vector3 &position);
  void setRotation(const Quaternion &rotation);
  void setScale(const Vector3 &scale);

  void translate(const Vector3 &delta);
  void rotate(const Quaternion &rotation);
  void scale(const Vector3 &scale);

  // Getters
  const Vector3 &getPosition() const { return _position; }
  const Quaternion &getRotation() const { return _rotation; }
  const Vector3 &getScale() const { return _scale; }

  // World matrix calculation
  const Matrix4 &getWorldMatrix() const;
  void setWorldMatrix(const Matrix4 &matrix);

  // Modern change tracking system
  bool hasChanged() const { return _changeId != _lastObservedChangeId; }
  void markDirty() { ++_changeId; }
  void clearDirty() { _lastObservedChangeId = _changeId; }

  // For debugging/profiling
  uint32 getChangeId() const { return _changeId; }

  // Update for compatibility with GameObject update
  void update(float32 dt) { (void)dt; /* no-op */ }

  // Compatibility methods for Frustrum and Camera
  Vector3 getForward() const { /* assume default forward */ return Vector3(0, 0, -1); }
  Vector3 getUp() const { /* default up */ return Vector3(0, 1, 0); }
  Vector3 getRight() const { /* default right */ return Vector3(1, 0, 0); }
  bool isAxisAligned() const { return true; }

  // Parent-child relationships (for hierarchy)
  void setParent(TransformComponent *parent);
  TransformComponent *getParent() const { return _parent; }

protected:
  // Override ComponentBase hook to add initialization logic
  void onInitialize() override;

private:
  Vector3 _position = Vector3::Zero;
  Quaternion _rotation = Quaternion::Identity;
  Vector3 _scale = Vector3::Identity;

  // UI editing state - keep separate from actual rotation to prevent coupling
  mutable Vector3 _uiEulerAngles = Vector3::Zero; // Stored in degrees
  mutable bool _uiEulerValid = false;             // Whether UI angles are in sync with quaternion

  mutable Matrix4 _worldMatrix = Matrix4::Identity;
  mutable uint32 _lastMatrixChangeId = 0;   // Last change when matrix was calculated
  uint32 _changeId = 1;                     // Incremented on each change (start at 1 so initial state is dirty)
  mutable uint32 _lastObservedChangeId = 0; // For hasChanged() tracking
  mutable uint32 _lastObservedParentChangeId = 0; // For parent change tracking

  TransformComponent *_parent = nullptr;

  void updateWorldMatrix() const;
};

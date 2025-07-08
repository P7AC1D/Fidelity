#pragma once

#include <functional>
#include <vector>
#include "IComponent.h"
#include "ComponentTypeId.h"
#include "../Core/Maths.h"
#include "../Maths/Quaternion.hpp"
#include "Types.hpp"

/// Modern Transform component that implements IComponent interface.
/// This will eventually replace the old Transform class for new GameObjects.
class TransformComponent : public IComponent
{
public:
  TransformComponent();
  TransformComponent(const Vector3 &position, const Quaternion &rotation = Quaternion::Identity, const Vector3 &scale = Vector3::Identity);

  // IComponent interface
  void initialize() override;
  void activate() override;
  void deactivate() override;
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

  // Change tracking
  bool hasChanged() const { return _dirty; }
  void markDirty() { _dirty = true; }
  void clearDirty() { _dirty = false; }

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

  // Observer pattern for change notifications
  using ChangeCallback = std::function<void()>;

  void addChangeObserver(ChangeCallback callback);
  void removeChangeObserver(ChangeCallback callback);

private:
  Vector3 _position = Vector3::Zero;
  Quaternion _rotation = Quaternion::Identity;
  Vector3 _scale = Vector3::Identity;

  mutable Matrix4 _worldMatrix = Matrix4::Identity;
  mutable bool _worldMatrixDirty = true;
  bool _dirty = true;

  TransformComponent *_parent = nullptr;

  std::vector<ChangeCallback> _changeObservers;

  void updateWorldMatrix() const;
  void notifyChanged();
};

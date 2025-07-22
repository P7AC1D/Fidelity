#pragma once

#include "ComponentTypeId.h"
#include "Types.hpp"

// Forward declaration to avoid circular dependency
class GameObject;

/// IComponent is the modern base interface for all engine components.
/// It defines optional lifecycle methods and a type-safe identifier.
class IComponent
{
public:
    virtual ~IComponent() = default;

    /// Called once after creation; set up any required state here.
    virtual void initialize() {}

    /// Called when the component is activated (e.g. added to GameObject).
    virtual void activate() {}

    /// Called when the component is deactivated or removed.
    virtual void deactivate() {}

    /// Called every frame for component updates (replaces IUpdatableComponent).
    virtual void update(float32 dt) { (void)dt; /* Default: no-op */ }

    /// Returns the unique ComponentTypeId for this concrete component.
    virtual ComponentTypeId getTypeId() const = 0;

    /// Optional inspector hook for editor UI.
    virtual void drawInspector() {}

    /// Set the parent GameObject (called by GameObject when component is added)
    void setGameObject(GameObject* gameObject) { _gameObject = gameObject; }

    /// Get the parent GameObject
    GameObject* getGameObject() const { return _gameObject; }

protected:
    /// Pointer to the parent GameObject that owns this component
    GameObject* _gameObject = nullptr;
};

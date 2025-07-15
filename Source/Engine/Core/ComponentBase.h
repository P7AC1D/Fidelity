#pragma once

#include "IComponent.h"
#include "ComponentTypeId.h"
#include <memory>

// Forward declarations
class GameObject;

/// Abstract base class that provides common functionality for all engine components.
/// Consolidates lifecycle management and provides convenient helper methods.
/// This replaces the need for multiple interfaces (IComponent + IUpdatableComponent + IComponentDependency).
class ComponentBase : public IComponent
{
public:
    ComponentBase() = default;
    virtual ~ComponentBase() = default;

    // IComponent interface implementation with common patterns
    void initialize() override;
    void activate() override;
    void deactivate() override;
    
    // Update method - all components now have this (replaces IUpdatableComponent)
    void update(float32 dt) override;

    // Common utility methods
    bool isActive() const { return _active; }
    bool isInitialized() const { return _initialized; }

protected:
    // Virtual hooks for derived classes to override (optional)
    virtual void onInitialize() {}
    virtual void onActivate() {}
    virtual void onDeactivate() {}
    virtual void onUpdate(float32 dt) { (void)dt; /* Default: no-op */ }

    // Helper methods for querying other components from parent GameObject
    // This replaces the ComponentDependency system with simple direct queries
    template<typename T>
    T* getComponent() const;
    
    template<typename T>
    std::shared_ptr<T> getComponentShared() const;

private:
    bool _active = false;
    bool _initialized = false;
};

// Template implementations - included here for template instantiation
// Forward declaration is sufficient for the class definition above
#include "GameObject.h"



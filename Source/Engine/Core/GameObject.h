#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include "IComponent.h"
#include "ComponentManager.h"
#include "ComponentTypeId.h"
#include "TransformComponent.h"
#include "Types.hpp"
#include "ComponentDependency.h"

/// Modern GameObject implementation with type-safe component management.
/// This is the new version that will eventually replace the old GameObject.
class ComponentDependencyResolver;

class GameObject
{
    friend class ComponentDependencyResolver;
    
public:
    GameObject(const std::string& name, uint64 index, ComponentManager* componentManager);
    ~GameObject();

    /// Add a component of type T with given constructor arguments.
    template<typename T, typename... Args>
    T& addComponent(Args&&... args);

    /// Get a component of type T. Throws if not found.
    template<typename T>
    T& getComponent();

    /// Get a component of type T. Returns nullptr if not found.
    template<typename T>
    T* tryGetComponent();

    /// Check if this GameObject has a component of type T.
    template<typename T>
    bool hasComponent() const;

    /// Remove a component of type T.
    template<typename T>
    bool removeComponent();

    /// Add a child GameObject.
    GameObject& addChild(std::unique_ptr<GameObject> child);

    /// Get the transform component (always present).
    TransformComponent& transform() { return *_transform; }
    const TransformComponent& transform() const { return *_transform; }

    /// Update this GameObject and all its children.
    void update(float32 dt);

    /// Draw inspector UI for this GameObject.
    void drawInspector();

    /// Getters
    const std::string& getName() const { return _name; }
    uint64 getIndex() const { return _index; }
    GameObject* getParent() const { return _parent; }
    const std::vector<std::unique_ptr<GameObject>>& getChildren() const { return _children; }

    /// Activate/deactivate this GameObject.
    void setActive(bool active);
    bool isActive() const { return _active; }

private:
    std::string _name;
    uint64 _index;
    bool _active = true;

    GameObject* _parent = nullptr;
    std::vector<std::unique_ptr<GameObject>> _children;

    ComponentManager* _componentManager;
    std::unordered_map<ComponentTypeId, std::unique_ptr<IComponent>> _components;

    // Transform is always present
    TransformComponent* _transform;

    void onActivated();
    void onDeactivated();
};

// Template implementations
template<typename T, typename... Args>
T& GameObject::addComponent(Args&&... args)
{
    static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");

    ComponentTypeId typeId = getComponentTypeId<T>();

    // Check if component already exists
    if (_components.find(typeId) != _components.end())
    {
        throw std::runtime_error("Component of this type already exists on GameObject");
    }

    // Create the component
    auto component = _componentManager->createComponent<T>(std::forward<Args>(args)...);
    T* componentPtr = component.get();

    // Store it
    _components[typeId] = std::move(component);

    // Activate if GameObject is active
    if (_active)
    {
        componentPtr->activate();
    }

    // Resolve dependencies automatically
    ComponentDependencyResolver::resolveDependencies(*this);

    return *componentPtr;
}

template<typename T>
T& GameObject::getComponent()
{
    static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");

    ComponentTypeId typeId = getComponentTypeId<T>();
    auto it = _components.find(typeId);

    if (it == _components.end())
    {
        throw std::runtime_error("Component of this type does not exist on GameObject");
    }

    return static_cast<T&>(*it->second);
}

template<typename T>
T* GameObject::tryGetComponent()
{
    static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");

    ComponentTypeId typeId = getComponentTypeId<T>();
    auto it = _components.find(typeId);

    if (it == _components.end())
    {
        return nullptr;
    }

    return static_cast<T*>(it->second.get());
}

template<typename T>
bool GameObject::hasComponent() const
{
    static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");

    ComponentTypeId typeId = getComponentTypeId<T>();
    return _components.find(typeId) != _components.end();
}

template<typename T>
bool GameObject::removeComponent()
{
    static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");

    ComponentTypeId typeId = getComponentTypeId<T>();
    auto it = _components.find(typeId);

    if (it == _components.end())
    {
        return false;
    }

    // Deactivate before removal
    it->second->deactivate();

    // Remove from storage
    _components.erase(it);

    return true;
}

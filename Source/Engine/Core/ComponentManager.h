#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>

#include "IComponent.h"
#include "ComponentTypeId.h"
#include "ComponentDependency.h"

class GameObject;

/// ComponentManager handles component lifecycle, dependencies, and queries.
class ComponentManager
{
public:
    ComponentManager() = default;
    ~ComponentManager() = default;

    /// Register a component factory for type T.
    template<typename T>
    void registerComponentType();

    /// Create a component of type T with given arguments.
    template<typename T, typename... Args>
    std::unique_ptr<T> createComponent(Args&&... args);

    /// Resolve dependencies for all components on a GameObject.
    void resolveDependencies(GameObject& gameObject);

    /// Get all components of type T from all GameObjects.
    template<typename T>
    std::vector<T*> getComponentsOfType();

    /// Query GameObjects that have all specified component types.
    template<typename... ComponentTypes>
    std::vector<GameObject*> getEntitiesWith();

private:
    /// Component factory function signature.
    using ComponentFactory = std::function<std::unique_ptr<IComponent>()>;

    /// Map of component type IDs to their factory functions.
    std::unordered_map<ComponentTypeId, ComponentFactory> _componentFactories;

    /// Map of component type IDs to all instances of that type.
    std::unordered_map<ComponentTypeId, std::vector<IComponent*>> _componentInstances;

    /// Register an instance for querying.
    void registerInstance(IComponent* component);

    /// Unregister an instance.
    void unregisterInstance(IComponent* component);
};

// Template implementations
template<typename T>
void ComponentManager::registerComponentType()
{
    static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");
    
    ComponentTypeId typeId = getComponentTypeId<T>();
    _componentFactories[typeId] = []() -> std::unique_ptr<IComponent> {
        return std::make_unique<T>();
    };
}

template<typename T, typename... Args>
std::unique_ptr<T> ComponentManager::createComponent(Args&&... args)
{
    static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");
    
    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    registerInstance(component.get());
    component->initialize();
    
    return component;
}

template<typename T>
std::vector<T*> ComponentManager::getComponentsOfType()
{
    static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");
    
    std::vector<T*> result;
    ComponentTypeId typeId = getComponentTypeId<T>();
    
    auto it = _componentInstances.find(typeId);
    if (it != _componentInstances.end())
    {
        for (IComponent* component : it->second)
        {
            result.push_back(static_cast<T*>(component));
        }
    }
    
    return result;
}

template<typename... ComponentTypes>
std::vector<GameObject*> ComponentManager::getEntitiesWith()
{
    // This would need GameObject reference tracking
    // Implementation would iterate through all GameObjects
    // and check if they have all specified component types
    // For now, return empty vector as placeholder
    return {};
}

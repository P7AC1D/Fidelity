#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include "ComponentTypeId.h"

class IComponent;
class GameObject;

/// Interface for components that have dependencies on other components
class IComponentDependency
{
public:
    virtual ~IComponentDependency() = default;
    
    /// Returns the list of component types this component depends on
    virtual std::vector<ComponentTypeId> getDependencies() const = 0;
    
    /// Called when dependencies are resolved
    virtual void onDependenciesResolved(GameObject& gameObject) = 0;
};

/// Helper class to automatically manage component dependencies
class ComponentDependencyResolver
{
public:
    /// Register dependency resolution for a specific component type
    template<typename ComponentType, typename DependencyType>
    static void registerDependency();
    
    /// Resolve all dependencies for a GameObject
    static void resolveDependencies(GameObject& gameObject);
    
    /// Resolve dependencies for a specific component (optimized)
    static void resolveDependenciesForComponent(GameObject& gameObject, IComponent* newComponent);
    
    /// Check if a component type has dependencies
    template<typename ComponentType>
    static bool hasDependencies();
    
private:
    /// Dependency resolution function signature
    using DependencyResolver = std::function<void(GameObject&, IComponent*)>;
    
    /// Map of component type IDs to their dependency resolvers
    static std::unordered_map<ComponentTypeId, std::vector<DependencyResolver>>& getDependencyMap();
};

/// Template helper to automatically resolve specific dependencies
template<typename ComponentType, typename DependencyType>
class AutoDependency
{
public:
    static void resolve(GameObject& gameObject, ComponentType* component);
};

// Template implementations
template<typename ComponentType, typename DependencyType>
void ComponentDependencyResolver::registerDependency()
{
    static_assert(std::is_base_of_v<IComponent, ComponentType>, "ComponentType must derive from IComponent");
    static_assert(std::is_base_of_v<IComponent, DependencyType>, "DependencyType must derive from IComponent");
    
    ComponentTypeId componentTypeId = getComponentTypeId<ComponentType>();
    
    auto resolver = [](GameObject& gameObject, IComponent* component) {
        auto* typedComponent = static_cast<ComponentType*>(component);
        AutoDependency<ComponentType, DependencyType>::resolve(gameObject, typedComponent);
    };
    
    getDependencyMap()[componentTypeId].push_back(resolver);
}

template<typename ComponentType>
bool ComponentDependencyResolver::hasDependencies()
{
    ComponentTypeId componentTypeId = getComponentTypeId<ComponentType>();
    auto& depMap = getDependencyMap();
    return depMap.find(componentTypeId) != depMap.end();
}

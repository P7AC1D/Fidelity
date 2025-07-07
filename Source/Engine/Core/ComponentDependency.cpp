#include "ComponentDependency.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "../Rendering/CameraComponent.h"
#include "../Rendering/LightComponent.h"

std::unordered_map<ComponentTypeId, std::vector<ComponentDependencyResolver::DependencyResolver>>& 
ComponentDependencyResolver::getDependencyMap()
{
    static std::unordered_map<ComponentTypeId, std::vector<DependencyResolver>> dependencyMap;
    return dependencyMap;
}

void ComponentDependencyResolver::resolveDependencies(GameObject& gameObject)
{
    auto& dependencyMap = getDependencyMap();
    
    // Iterate through all components on the GameObject
    for (auto& [typeId, component] : gameObject._components)
    {
        // Check if this component type has registered dependencies
        auto it = dependencyMap.find(typeId);
        if (it != dependencyMap.end())
        {
            // Execute all dependency resolvers for this component
            for (auto& resolver : it->second)
            {
                resolver(gameObject, component.get());
            }
        }
    }
}

// Specialized dependency resolution for CameraComponent -> TransformComponent
template<>
void AutoDependency<CameraComponent, TransformComponent>::resolve(GameObject& gameObject, CameraComponent* camera)
{
    auto* transform = gameObject.tryGetComponent<TransformComponent>();
    if (transform)
    {
        // Create a shared_ptr that doesn't own the object (since GameObject owns it)
        auto transformPtr = std::shared_ptr<TransformComponent>(transform, [](TransformComponent*){});
        camera->setTransformComponent(transformPtr);
    }
}

// Specialized dependency resolution for LightComponent -> TransformComponent
template<>
void AutoDependency<LightComponent, TransformComponent>::resolve(GameObject& gameObject, LightComponent* light)
{
    auto* transform = gameObject.tryGetComponent<TransformComponent>();
    if (transform)
    {
        // Create a shared_ptr that doesn't own the object (since GameObject owns it)
        auto transformPtr = std::shared_ptr<TransformComponent>(transform, [](TransformComponent*){});
        light->setTransformComponent(transformPtr);
    }
}

// Register dependency functions - called during static initialization
namespace
{
    struct DependencyRegistrar
    {
        DependencyRegistrar()
        {
            // Register CameraComponent's dependency on TransformComponent
            ComponentDependencyResolver::registerDependency<CameraComponent, TransformComponent>();
            
            // Register LightComponent's dependency on TransformComponent
            ComponentDependencyResolver::registerDependency<LightComponent, TransformComponent>();
        }
    };
    
    // Static instance to ensure registration happens
    static DependencyRegistrar registrar;
}

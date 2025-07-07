#include "ComponentDependency.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "../Rendering/CameraComponent.h"
#include "../Rendering/LightComponent.h"
#include "../Rendering/DrawableComponent.h"

std::unordered_map<ComponentTypeId, std::vector<ComponentDependencyResolver::DependencyResolver>> &
ComponentDependencyResolver::getDependencyMap()
{
  static std::unordered_map<ComponentTypeId, std::vector<DependencyResolver>> dependencyMap;
  return dependencyMap;
}

void ComponentDependencyResolver::resolveDependencies(GameObject &gameObject)
{
  auto &dependencyMap = getDependencyMap();

  // Iterate through all components on the GameObject
  for (auto &[typeId, component] : gameObject._components)
  {
    // Check if this component implements IComponentDependency interface
    if (auto *dependencyComponent = dynamic_cast<IComponentDependency *>(component.get()))
    {
      // Call the interface method
      dependencyComponent->onDependenciesResolved(gameObject);
    }

    // Also check if this component type has registered dependencies (for AutoDependency system)
    auto it = dependencyMap.find(typeId);
    if (it != dependencyMap.end())
    {
      // Execute all dependency resolvers for this component
      for (auto &resolver : it->second)
      {
        resolver(gameObject, component.get());
      }
    }
  }
}

// Specialized dependency resolution for CameraComponent -> TransformComponent
template <>
void AutoDependency<CameraComponent, TransformComponent>::resolve(GameObject &gameObject, CameraComponent *camera)
{
  if (auto transformShared = gameObject.getComponentShared<TransformComponent>())
  {
    // Convert to weak_ptr for the camera
    std::weak_ptr<TransformComponent> weakPtr = transformShared;
    camera->setTransformComponent(weakPtr);
  }
}

// Specialized dependency resolution for LightComponent -> TransformComponent
template <>
void AutoDependency<LightComponent, TransformComponent>::resolve(GameObject &gameObject, LightComponent *light)
{
  if (auto transformShared = gameObject.getComponentShared<TransformComponent>())
  {
    // Convert to weak_ptr for the light
    std::weak_ptr<TransformComponent> weakPtr = transformShared;
    light->setTransformComponent(weakPtr);
  }
}

// Specialized dependency resolution for DrawableComponent -> TransformComponent
template <>
void AutoDependency<DrawableComponent, TransformComponent>::resolve(GameObject &gameObject, DrawableComponent *drawable)
{
  if (auto transformShared = gameObject.getComponentShared<TransformComponent>())
  {
    // Convert to weak_ptr for the drawable
    std::weak_ptr<TransformComponent> weakPtr = transformShared;
    drawable->setTransformComponent(weakPtr);
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

      // Register DrawableComponent's dependency on TransformComponent
      ComponentDependencyResolver::registerDependency<DrawableComponent, TransformComponent>();
    }
  };

  // Static instance to ensure registration happens
  static DependencyRegistrar registrar;
}

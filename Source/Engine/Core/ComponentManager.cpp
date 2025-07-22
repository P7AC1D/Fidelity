#include <algorithm>

#include "ComponentManager.h"
#include "GameObject.h"

void ComponentManager::resolveDependencies(GameObject& gameObject)
{
    // This is for the old GameObject system - kept for compatibility
    // For GameObjectV2, dependencies are automatically resolved
    // through ComponentDependencyResolver
}

void ComponentManager::registerInstance(IComponent* component)
{
    if (component)
    {
        ComponentTypeId typeId = component->getTypeId();
        _componentInstances[typeId].push_back(component);
    }
}

void ComponentManager::unregisterInstance(IComponent* component)
{
    if (component)
    {
        ComponentTypeId typeId = component->getTypeId();
        auto& instances = _componentInstances[typeId];
        
        auto it = std::find(instances.begin(), instances.end(), component);
        if (it != instances.end())
        {
            instances.erase(it);
        }
    }
}

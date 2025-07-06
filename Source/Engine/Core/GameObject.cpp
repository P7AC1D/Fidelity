#include "GameObject.h"

GameObject::GameObject(const std::string& name, uint64 index, ComponentManager* componentManager)
    : _name(name)
    , _index(index)
    , _componentManager(componentManager)
{
    // Every GameObject has a Transform component by default
    // For now, we'll create it manually since Transform isn't modernized yet
    // This will be updated when we modernize the Transform component
    _transform = new TransformComponent();
}

GameObject::~GameObject()
{
    // Deactivate all components before destruction
    for (auto& [typeId, component] : _components)
    {
        component->deactivate();
    }

    // Clean up transform (temporary)
    delete _transform;
}

GameObject& GameObject::addChild(std::unique_ptr<GameObject> child)
{
    child->_parent = this;
    GameObject& childRef = *child;
    _children.push_back(std::move(child));
    return childRef;
}

void GameObject::update(float32 dt)
{
    if (!_active)
    {
        return;
    }

    // Update transform
    _transform->update(dt);

    // Update all children
    for (auto& child : _children)
    {
        child->update(dt);
    }
}

void GameObject::drawInspector()
{
    // Basic inspector implementation
    // This would be expanded with ImGui calls similar to the old GameObject
    
    // Draw name
    // ImGui::Text("Name: %s", _name.c_str());
    
    // Draw active checkbox
    // bool active = _active;
    // if (ImGui::Checkbox("Active", &active))
    // {
    //     setActive(active);
    // }
    
    // Draw transform
    // _transform->drawInspector(); // Would need to be implemented
    
    // Draw all components
    for (auto& [typeId, component] : _components)
    {
        component->drawInspector();
    }
}

void GameObject::setActive(bool active)
{
    if (_active == active)
    {
        return;
    }

    _active = active;

    if (_active)
    {
        onActivated();
    }
    else
    {
        onDeactivated();
    }

    // Propagate to children
    for (auto& child : _children)
    {
        child->setActive(active);
    }
}

void GameObject::onActivated()
{
    // Activate all components
    for (auto& [typeId, component] : _components)
    {
        component->activate();
    }
}

void GameObject::onDeactivated()
{
    // Deactivate all components
    for (auto& [typeId, component] : _components)
    {
        component->deactivate();
    }
}

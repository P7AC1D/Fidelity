#include "GameObject.h"
#include "IUpdatableComponent.h"
#include "../UI/ImGui/imgui.h"

GameObject::GameObject(const std::string &name, uint64 index, ComponentManager *componentManager)
    : _name(name), _index(index), _componentManager(componentManager)
{
  addComponent<TransformComponent>(); // Always add a transform component
}

GameObject::~GameObject()
{
  // Deactivate all components before destruction
  for (auto &[typeId, component] : _components)
  {
    component->deactivate();
  }
}

GameObject &GameObject::addChild(std::unique_ptr<GameObject> child)
{
  child->_parent = this;
  GameObject &childRef = *child;
  _children.push_back(std::move(child));
  return childRef;
}

void GameObject::update(float32 dt)
{
  if (!_active)
  {
    return;
  }
  
  // Update all components
  for (auto &[typeId, component] : _components)
  {
    // Check if component has an update method (we'll add this interface)
    if (auto* updatableComponent = dynamic_cast<IUpdatableComponent*>(component.get()))
    {
      updatableComponent->update(dt);
    }
  }
  
  // Clear transform dirty flag after all components have been updated
  if (auto* transform = tryGetComponent<TransformComponent>())
  {
    transform->clearDirty();
  }
  
  // Update all children
  for (auto &child : _children)
  {
    child->update(dt);
  }
}

void GameObject::drawInspector()
{
  // Draw all components
  for (auto &[typeId, component] : _components)
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
  for (auto &child : _children)
  {
    child->setActive(active);
  }
}

void GameObject::onActivated()
{
  // Activate all components
  for (auto &[typeId, component] : _components)
  {
    component->activate();
  }
}

void GameObject::onDeactivated()
{
  // Deactivate all components
  for (auto &[typeId, component] : _components)
  {
    component->deactivate();
  }
}

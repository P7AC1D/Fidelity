#include "GameObject.h"

#include "Component.h"

GameObject::GameObject(const std::string &name) : _name(name)
{
}

void GameObject::update(float32 dt)
{
  if (_transform.modified())
  {
    _transform.update(dt);
    notifyComponents();
  }
}

GameObject &GameObject::addChild(std::shared_ptr<GameObject> child)
{
  _children.push_back(child);
  return *this;
}

GameObject &GameObject::addComponent(std::shared_ptr<Component> component)
{
  _components.push_back(component);
  return *this;
}

void GameObject::notifyComponents() const
{
  for (auto component : _components)
  {
    component->notify(*this);
  }
}
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

GameObject &GameObject::setParent(const GameObject &parent)
{
  _parent = &parent;
  return *this;
}

GameObject &GameObject::addChild(GameObject &child)
{
  _children.push_back(&child);
  return *this;
}

GameObject &GameObject::addComponent(Component &component)
{
  _components.push_back(&component);
  return *this;
}

void GameObject::notifyComponents() const
{
  for (auto component : _components)
  {
    component->notify(*this);
  }
}
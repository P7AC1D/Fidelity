#include "GameObject.h"

#include "Component.h"

GameObject::GameObject(const std::string &name, uint64 index) : _name(name), _index(index)
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

GameObject &GameObject::addComponent(Component &component)
{
  _components.push_back(component);
  return *this;
}

void GameObject::notifyComponents() const
{
  for (auto component : _components)
  {
    component.get().notify(*this);
  }
}
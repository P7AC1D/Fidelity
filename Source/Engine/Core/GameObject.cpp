#include "GameObject.h"

GameObject::GameObject(const std::string &name) : _name(name)
{
}

void GameObject::update(float32 dt)
{
  if (_transform.modified())
  {
    _transform.update(dt);
  }
  onUpdate(dt);
}
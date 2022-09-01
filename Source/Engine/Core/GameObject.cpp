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

  for (auto child : _children)
  {
    child->update(dt);
  }
}

void GameObject::setParent(const GameObject &parent)
{
  _parent = &parent;
}

void GameObject::addChild(GameObject &child)
{
  _children.push_back(&child);
}
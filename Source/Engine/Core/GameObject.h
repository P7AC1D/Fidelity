#pragma once
#include <functional>
#include <string>
#include <list>
#include <stdexcept>

#include "Component.h"
#include "Transform.h"
#include "Types.hpp"

class GameObject
{
public:
  GameObject();
  GameObject(const std::string &name, uint64 index);

  void update(float32 dt);
  void drawInspector();

  GameObject &addComponent(Component &component);
  template <typename T>
  T& getComponent();
  template <typename T>
  bool hasComponent();

  Transform &transform() { return _transform; }
  const Transform &getTransform() const { return _transform; }

  std::string getName() const { return _name; }

  uint64 getIndex() const { return _index; }

protected:
  Transform _transform;
  std::string _name;
  uint64 _index;

private:
  void notifyComponents() const;

  std::list<Component*> _components;
};

template <typename T>
T& GameObject::getComponent()
{
  std::list<Component*>::iterator iter = std::find_if(_components.begin(), _components.end(), [](Component* c)
    {
      return dynamic_cast<T*>(c) != nullptr;
    });

  if (iter == _components.end())
  {
    throw std::runtime_error("Component type does not exist.");
  }

  return *dynamic_cast<T*>(*iter);
}

template <typename T>
bool GameObject::hasComponent()
{
  auto iter = std::find_if(_components.begin(), _components.end(), [](Component* c)
    {
      return dynamic_cast<T*>(c) != nullptr;
    });
  return iter != _components.end();
}
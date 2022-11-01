#pragma once
#include <functional>
#include <string>
#include <list>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Component.h"
#include "Transform.h"
#include "Types.hpp"

static uint64 GAMEOBJECT_ID = 0;

class GameObject
{
public:
  GameObject();
  GameObject(const std::string &name);

  void update(float32 dt);
  void drawInspector();

  std::shared_ptr<GameObject> createChildNode(const std::string &name);
  std::shared_ptr<GameObject> addChildNode(std::shared_ptr<GameObject> gameObject);
  std::shared_ptr<GameObject> getChildNode(uint64 id) const;
  const std::vector<std::shared_ptr<GameObject>> &getChildNodes() const { return _children; }
  bool hasChildNodes() const { return !_children.empty(); }
  void removeChildNode(uint64 id);

  template <typename T, typename... Args>
  std::shared_ptr<T> createComponent(Args... args);
  template <typename T, typename... Args>
  std::shared_ptr<T> addComponent(std::shared_ptr<T> component);
  template <typename T>
  std::shared_ptr<T> getComponent() const;
  template <typename T>
  bool hasComponent() const;
  template <typename T>
  void removeComponent();

  Transform &transform() { return _localTransform; }
  const Transform &getLocalTransform() const { return _localTransform; }
  const Transform &getGlobalTransform() const { return _globalTransform; }

  std::string getName() const { return _name; }
  uint64 getId() const { return _id; }

protected:
  Transform _localTransform, _globalTransform;
  GameObject *_parent;
  std::vector<std::shared_ptr<GameObject>> _children;
  std::vector<std::shared_ptr<Component>> _components;
  std::string _name;
  uint64 _id;
  uint32 _componentTypes;

private:
  void updateChildNodeTransforms(float32 dt);
  void notifyComponents() const;
  void updateComponents(float32 dt);
  void updateChildren(float32 dt);
};

template <typename T, typename... Args>
std::shared_ptr<T> GameObject::createComponent(Args... args)
{
  static_assert(std::is_base_of<Component, T>::value, "Type is not derived from Component.");
  std::shared_ptr<T> component(new T(args...));
  addComponent<T>(component);
  return component;
}

template <typename T, typename... Args>
std::shared_ptr<T> GameObject::addComponent(std::shared_ptr<T> component)
{
  static_assert(std::is_base_of<Component, T>::value, "Type is not derived from Component.");
  auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component> &c)
                           { return std::dynamic_pointer_cast<T>(c) != nullptr; });

  if (iter != _components.end())
  {
    *iter = component;
  }
  _components.push_back(component);
  return component;
}

template <typename T>
std::shared_ptr<T> GameObject::getComponent() const
{
  auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component> &c)
                           { return std::dynamic_pointer_cast<T>(c) != nullptr; });

  return iter != _components.end() ? std::static_pointer_cast<T>(*iter) : nullptr;
}

template <typename T>
bool GameObject::hasComponent() const
{
  auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component> &c)
                           { return std::dynamic_pointer_cast<T>(c) != nullptr; });
  return iter != _components.end();
}

template <typename T>
void GameObject::removeComponent()
{
  auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component> &c)
                           { return std::dynamic_pointer_cast<T>(c) != nullptr; });
  if (iter != _components.end())
  {
    _components.erase(iter);
  }
}
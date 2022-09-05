#pragma once
#include <string>
#include <vector>

#include "Transform.h"
#include "Types.hpp"

class Component;

class GameObject
{
public:
  GameObject(const std::string &name);

  void update(float32 dt);

  GameObject &addChild(std::shared_ptr<GameObject> child);
  GameObject &addComponent(std::shared_ptr<Component> component);

  Transform &transform() { return _transform; }
  const Transform &getTransform() const { return _transform; }

  std::string getName() const { return _name; }

protected:
  Transform _transform;
  std::string _name;

private:
  void notifyComponents() const;

  std::vector<std::shared_ptr<GameObject>> _children;
  std::vector<std::shared_ptr<Component>> _components;
};
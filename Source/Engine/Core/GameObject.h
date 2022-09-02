#pragma once
#include <string>
#include <vector>

#include "Transform.h"
#include "Types.hpp"

class Component;

class GameObject
{
public:
  void update(float32 dt);

  GameObject &setParent(const GameObject &parent);
  GameObject &addChild(GameObject &child);
  GameObject &addComponent(Component &component);

  Transform &transform() { return _transform; }
  const Transform &getTransform() const { return _transform; }

  std::string getName() const { return _name; }

protected:
  GameObject(const std::string &name);

  Transform _transform;
  std::string _name;

private:
  virtual void onUpdate(float32 dt) = 0;

  void notifyComponents() const;

  const GameObject *_parent;
  std::vector<GameObject *> _children;
  std::vector<Component *> _components;
};
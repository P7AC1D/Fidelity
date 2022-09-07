#pragma once
#include <functional>
#include <string>
#include <vector>

#include "Transform.h"
#include "Types.hpp"

class Component;

class GameObject
{
public:
  GameObject(const std::string &name, uint64 index);

  void update(float32 dt);
  void updateInspector() {}

  GameObject &addComponent(Component &component);

  Transform &transform() { return _transform; }
  const Transform &getTransform() const { return _transform; }

  std::string getName() const { return _name; }

  uint64 getIndex() { return _index; }

protected:
  Transform _transform;
  std::string _name;
  uint64 _index;

private:
  void notifyComponents() const;

  std::vector<std::reference_wrapper<Component>> _components;
};
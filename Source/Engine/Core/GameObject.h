#pragma once
#include <string>

#include "Transform.h"
#include "Types.hpp"

class GameObject
{
public:
  virtual void onUpdate(float32 dt) = 0;

  void update(float32 dt);

  Transform &transform() { return _transform; }
  const Transform &getTransform() const { return _transform; }

protected:
  GameObject(const std::string &name);

  Transform _transform;
  std::string _name;
};
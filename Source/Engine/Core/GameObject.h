#pragma once
#include <string>

#include "Transform.h"
#include "Types.h"

class GameObject
{
public:
  virtual void update(float32 dt);
  virtual void drawInspector();

  std::string getName() const { return _name; }
  Transform &getTransform() const { return _transform; }

  void setName(const std::string &name) { _name = name; }

private:
  std::string _name;
  Transform _transform;
};
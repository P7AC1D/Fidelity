#pragma once
#include <string>

#include "Transform.h"
#include "Types.hpp"

class GameObject : public Transform
{
public:
  virtual void update(float32 dt) = 0;
  virtual void drawInspector() = 0;

  std::string getName() const { return _name; }

  void setName(const std::string &name) { _name = name; }

protected:
  std::string _name;
};
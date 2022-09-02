#pragma once
#include <string>

#include "Types.hpp"

class GameObject;

class Component
{
public:
  void update(float32 dt);

  void notify(const GameObject &gameObject);

  std::string getName() const { return _name; }

protected:
  Component(const std::string &name);

  virtual void onUpdate(float32 dt) = 0;
  virtual void onNotify(const GameObject &gameObject) = 0;

  std::string _name;
};
#pragma once
#include <string>

#include "Types.hpp"

class GameObject;

enum class ComponentType
{
  Drawable,
  Light,
  Generic
};

class Component
{
public:
  void update(float32 dt);

  void notify(const GameObject &gameObject);

  ComponentType getType() const { return _componentType; }

protected:
  Component(ComponentType componentType);

  virtual void onUpdate(float32 dt) = 0;
  virtual void onNotify(const GameObject &gameObject) = 0;

  ComponentType _componentType;
};
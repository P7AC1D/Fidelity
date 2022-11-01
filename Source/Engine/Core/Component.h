#pragma once
#include <string>

#include "Types.hpp"

class GameObject;

class Component
{
public:
  void update(float32 dt);

  void notify(const GameObject &gameObject);

  virtual void drawInspector() = 0;

protected:
  Component();

  virtual void onUpdate(float32 dt) = 0;
  virtual void onNotify(const GameObject &gameObject) = 0;
};
#pragma once
#include <memory>

class Component;

class ComponentBuilder
{
public:
  virtual std::shared_ptr<Component> build() = 0;
};
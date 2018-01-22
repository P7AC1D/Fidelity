#pragma once
#include <string>

class Component
{
public:
  Component(std::string name) : _name(std::move(name)) {}
  virtual ~Component() {}

  std::string GetName() const { return _name; }

private:
  std::string _name;
};
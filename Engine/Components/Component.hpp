#pragma once
#include "Types.h"

#include <string>

class GameObject;

class Component
{
public:
  inline const std::string& GetComponentType() const { return _type; }
  inline uint64 GetId() const { return _id; }
  
  virtual void Update(uint32 dtMs) const {}
  
protected:
  Component(const std::string& type, const GameObject& parent, uint64 id):
  _type(type),
  _id(id),
  _parent(parent)
  {
  }
  
private:
  std::string _type;
  uint64 _id;
  const GameObject& _parent;
};

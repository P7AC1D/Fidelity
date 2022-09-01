#pragma once
#include <list>
#include <string>

#include "Transform.h"
#include "Types.hpp"

class GameObject
{
public:
  virtual void onUpdate(float32 dt) = 0;

  void update(float32 dt);

  void setParent(const GameObject &parent);
  void addChild(GameObject &child);

  Transform &transform() { return _transform; }
  const Transform &getTransform() const { return _transform; }

  std::string getName() const { return _name; }

protected:
  GameObject(const std::string &name);

  Transform _transform;
  std::string _name;

private:
  const GameObject *_parent;
  std::list<GameObject *> _children;
};
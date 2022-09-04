#include "Component.h"

#include "GameObject.h"

void Component::update(float32 dt)
{
  onUpdate(dt);
}

void Component::notify(const GameObject &gameObject)
{
  onNotify(gameObject);
}

Component::Component(ComponentType componentType) : _componentType(componentType)
{
}
#include "WorldObject.h"

#include "../Components/Component.h"
#include "../Rendering/Renderable.hpp"
#include "Transform.h"

WorldObject::WorldObject(const std::string& name) :
  _isDirty(true),
  _name(std::move(name)),
  _transform(new Transform)
{}

WorldObject::~WorldObject()
{}

void WorldObject::Update()
{
}

void WorldObject::AttachChild(WorldObject* childObject)
{
  childObject->_parent = this;
}

void WorldObject::AddComponent(std::shared_ptr<Component> component)
{
  _components[component->GetName()] = component;
}

bool WorldObject::HasComponent(const std::string& name) const
{
  auto iter = _components.find(name);
  return iter != _components.find(name);
}

std::shared_ptr<Component> WorldObject::GetComponent(const std::string& name) const
{
  auto iter = _components.find(name);
  if (iter == _components.end())
  {
    return nullptr;
  }
  return iter->second;
}

void WorldObject::SetPosition(const Vector3& position)
{
  _transform->SetPosition(position);
  _isDirty = true;
}

void WorldObject::SetScale(const Vector3& scale)
{
  _transform->SetScale(scale);
  _isDirty = true;
}

void WorldObject::SetRotation(const Quaternion& rotation)
{
  _transform->SetRotation(rotation);
  _isDirty = true;
}

void WorldObject::Rotate(const Quaternion& rotationDelta)
{
  _transform->Rotate(rotationDelta);
  _isDirty = true;
}

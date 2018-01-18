#include "WorldObject.h"

#include "../Components/Component.h"
#include "../Components/Transform.h"

using namespace Components;

WorldObject::WorldObject(const std::string& name) :
  _name(std::move(name)),
  _isDirty(true),
  _localTransform(new Transform),
  _absoluteTransform(new Transform)
{}

WorldObject::~WorldObject()
{}

void WorldObject::Update()
{
}

void WorldObject::AddComponent(std::shared_ptr<Component> component)
{
  auto transform = std::dynamic_pointer_cast<Transform>(component);
  if (transform)
  {
    _localTransform = transform;
    return;
  }
  _components[component->GetName()] = component;
}

bool WorldObject::HasComponent(const std::string& name) const
{
  auto iter = _components.find(name);
  return iter != _components.find(name);
}

std::shared_ptr<Component> WorldObject::GetComponent(const std::string& name)
{
  auto iter = _components.find(name);
  if (iter == _components.end())
  {
    return nullptr;
  }
  return iter->second;
}

Matrix4 WorldObject::GetTransform() const
{
  return _absoluteTransform->Get();
}

void WorldObject::SetPosition(const Vector3& position)
{
  _localTransform->SetPosition(position);
  _isDirty = true;
}

void WorldObject::SetScale(const Vector3& scale)
{
  _localTransform->SetScale(scale);
  _isDirty = true;
}

void WorldObject::SetRotation(const Quaternion& rotation)
{
  _localTransform->SetRotation(rotation);
  _isDirty = true;
}

void WorldObject::Rotate(const Quaternion& rotationDelta)
{
  _localTransform->Rotate(rotationDelta);
  _isDirty = true;
}
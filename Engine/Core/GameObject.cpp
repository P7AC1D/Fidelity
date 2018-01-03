#include "GameObject.hpp"

#include <algorithm>
#include <exception>
#include <sstream>

#include "Component.hpp"

GameObject::GameObject(const std::string& name):
_name(name),
_position(Vector3::Zero),
_scale(Vector3::Zero),
_orientation(Quaternion::Identity),
{
  static uint64 idCount = 0;
  _id = idCount;
  _components.resize(2);
}

void GameObject::AddComponent(Component&& componentToAdd)
{
  auto iter = std::find_if(_components.begin(), _components.end(), [&](const Component& component)
                           {
                             return component.GetComponentType() == componentToAdd.GetComponentType();
                           });
  if (iter == _components.end())
  {
    _components.push_back(std::move(componentToAdd));
  }
  else
  {
    *iter = std::move(componentToAdd);
  }
}

const Component& GameObject::GetComponent(const std::string& componentType)
{
  auto iter = std::find_if(_components.begin(), _components.end(), [&](const Component& component)
                           {
                             return strcmp(component.GetComponentType().c_str(), componentType.c_str()) == 0;
                           });
  if (iter == _components.end())
  {
    std::stringstream message;
    message << "Unable to find component '" << componentType << "' attached to game object " << GetId();
    throw std::runtime_error(message.str().c_str());
  }
  return *iter;
}

void GameObject::SetPosition(const Vector3& position)
{
  _position = position;
  _needsUpdate = true;
}

void GameObject::SetScale(const Vector3& scale)
{
  _scale = scale;
  _needsUpdate = true;
}

void GameObject::SetOrientation(const Quaternion& orientation)
{
  _orientation = orientation;
  _needsUpdate = true;
}

Matrix4 GameObject::GetTransform()
{
  if (_needsUpdate)
  {
    _transform = Matrix4::Transform(_position, _scale, _orientation);
    _needsUpdate = false;
  }
  return _transform;
}

void GameObject::Update(uint32 dtMs) const
{
  OnUpdate();
  for (auto& component : _components)
  {
    component.Update(dtMs);
  }
}

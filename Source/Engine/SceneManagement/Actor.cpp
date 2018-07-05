#include "Actor.hpp"

#include "SceneNode.hpp"
#include "Transform.h"

Actor::Actor(const std::string& name) : _name(name), _transform(new Transform())
{
}

void Actor::SetParent(const std::shared_ptr<SceneNode>& parent)
{
	_parent = parent;
}

void Actor::SetTransform(const std::shared_ptr<Transform>& transform)
{
	_transform = transform;
}

void Actor::SetPosition(const Vector3& position)
{
	_transform->SetPosition(position);
}

void Actor::SetScale(const Vector3& scale)
{
	_transform->SetScale(scale);
}

void Actor::SetRotation(const Quaternion& rotation)
{
	_transform->SetRotation(rotation);
}

void Actor::Rotate(const Quaternion& rotationDelta)
{
	_transform->Rotate(rotationDelta);
}

std::shared_ptr<Transform> Actor::GetTransform() const
{
	return _transform;
}
#include "Actor.hpp"

#include <algorithm>
#include "../Rendering/StaticMesh.h"
#include "../Rendering/Renderable.hpp"
#include "SceneManager.h"
#include "SceneNode.hpp"
#include "Transform.h"

Actor::Actor(const std::string& name) : _name(name), _transform(new Transform()), _boundsCalculated(false)
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

Aabb Actor::GetBounds()
{
	if (!_boundsCalculated)
	{
		CalculateBounds();
		_boundsCalculated = true;
	}
	return _bounds;
}

void Actor::CalculateBounds()
{
	auto renderable = GetComponent<Renderable>();
	if (renderable == nullptr)
	{
		return;
	}
	auto meshPositionData = renderable->GetMesh()->GetPositionVertexData();
	if (meshPositionData.empty())
	{
		return;
	}

	Vector3 max;
	Vector3 min;
	for (const auto& position : meshPositionData)
	{
		max.X = std::max(max.X, position.X);
		max.Y = std::max(max.Y, position.Y);
		max.Z = std::max(max.Z, position.Z);

		min.X = std::min(min.X, position.X);
		min.Y = std::min(min.Y, position.Y);
		min.Z = std::min(min.Z, position.Z);
	}

	auto transform = GetTransform();
	max += transform->GetPosition();
	min += transform->GetPosition();

	max *= transform->GetScale();
	min *= transform->GetScale();

	_bounds = Aabb(max, min);
}

void Actor::IfRenderableNotifySceneManager(const std::shared_ptr<Component>& component)
{
	auto renderable = std::dynamic_pointer_cast<Renderable>(component);
	if (renderable != nullptr)
	{
		SceneManager::Get()->PushRenderable(this);
	}
}

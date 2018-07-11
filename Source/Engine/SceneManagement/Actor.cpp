#include "Actor.hpp"

#include "../Rendering/StaticMesh.h"
#include "../Rendering/Renderable.hpp"
#include "SceneNode.hpp"
#include "Transform.h"

Actor::Actor(const std::string& name) : _name(name), _transform(new Transform()), _calculatedBounds(false)
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

void Actor::CalculateBounds()
{
	auto renderable = GetComponent<Renderable>();
	if (!renderable)
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

	_bounds.PositiveBounds = max;
	_bounds.NegativeBounds = min;

	_calculatedBounds = true;
}

Aabb Actor::GetBounds()
{
	if (!_calculatedBounds)
	{
		CalculateBounds();
	}
	return _bounds;
}
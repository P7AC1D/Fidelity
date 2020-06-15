#include "SceneNode.hpp"

#include "Actor.hpp"
#include "../Rendering/Renderer.h"

void SceneNode::Draw(std::shared_ptr<Renderer> renderer)
{
	for (const auto& child : _childNodes)
	{
		child->Draw(renderer);
	}
	OnDraw(renderer);
}

void SceneNode::Update(float64 dt)
{
	for (const auto& child : _childNodes)
	{
		child->Update(dt);
	}
	OnUpdate(dt);
}

Transform& SceneNode::GetTransform()
{
	return _transform;
}

void SceneNode::SetTransform(const Transform& transform)
{
	_transform = transform;
}
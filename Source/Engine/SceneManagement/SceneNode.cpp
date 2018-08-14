#include "SceneNode.hpp"

#include "Actor.hpp"
#include "Transform.h"

SceneNode::SceneNode(const std::string& name) : _name(name), _transform(new Transform())
{
}

std::shared_ptr<SceneNode> SceneNode::CreateChildNode(const std::string& name)
{
	std::shared_ptr<SceneNode> childNode(new SceneNode(name));
	childNode->SetParentNode(shared_from_this());
	_childNodes.push_back(childNode);
	return childNode;
}

std::shared_ptr<Actor> SceneNode::CreateActor(const std::string& name)
{
	std::shared_ptr<Actor> actor(new Actor(name));
	actor->SetParent(shared_from_this());
	_actors.push_back(actor);
	return actor;
}

void SceneNode::SetTransform(const std::shared_ptr<Transform>& transform)
{
	_transform = transform;
}

void SceneNode::SetParentNode(const std::shared_ptr<SceneNode>& parent)
{
	_parentNode = parent;
}

void SceneNode::AddChildNode(const std::shared_ptr<SceneNode>& child)
{
	_childNodes.push_back(child);
}

void SceneNode::AddActor(const std::shared_ptr<Actor>& actor)
{
	_actors.push_back(actor);
}

void SceneNode::SetPosition(const Vector3& position)
{
	_transform->SetPosition(position);
}

void SceneNode::SetScale(const Vector3& scale)
{
	_transform->SetScale(scale);
}

void SceneNode::SetRotation(const Quaternion& rotation)
{
	_transform->SetRotation(rotation);
}

void SceneNode::Rotate(const Quaternion& rotationDelta)
{
	_transform->Rotate(rotationDelta);
}

std::vector<std::shared_ptr<Actor>> SceneNode::GetActors() const
{
	return _actors;
}

std::vector<std::shared_ptr<SceneNode>> SceneNode::GetChildNodes() const
{
	return _childNodes;
}

std::vector<std::shared_ptr<Actor>> SceneNode::GetAllActors() const
{
	std::vector<std::shared_ptr<Actor>> actors(_actors);
	for (auto childNode : _childNodes)
	{
		auto childNodeActors = childNode->GetAllActors();
		actors.insert(actors.end(), childNodeActors.begin(), childNodeActors.end());
	}
	return actors;
}
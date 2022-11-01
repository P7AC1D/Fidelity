#include "GameObjectBuilder.h"

#include <algorithm>

#include "Component.h"
#include "GameObject.h"
#include "Scene.h"

GameObjectBuilder::GameObjectBuilder() : _scale(Vector3::Identity),
																				 _position(Vector3::Zero),
																				 _rotation(Quaternion::Identity)
{
}

GameObjectBuilder &GameObjectBuilder::withName(const std::string &name)
{
	_name = name;
	return *this;
}

GameObjectBuilder &GameObjectBuilder::withChild(const std::shared_ptr<GameObject> &childObject)
{
	_children.push_back(childObject);
	return *this;
}

GameObjectBuilder &GameObjectBuilder::withComponent(const std::shared_ptr<Component> &component)
{
	_components.push_back(component);
	return *this;
}

GameObjectBuilder &GameObjectBuilder::withScale(const Vector3 &scale)
{
	_scale = scale;
	return *this;
}

GameObjectBuilder &GameObjectBuilder::withPosition(const Vector3 &position)
{
	_position = position;
	return *this;
}

GameObjectBuilder &GameObjectBuilder::withTarget(const Vector3 &target)
{
	_target = target;
	return *this;
}

GameObjectBuilder &GameObjectBuilder::withRotation(const Quaternion &rotation)
{
	_rotation = rotation;
	return *this;
}

std::shared_ptr<GameObject> GameObjectBuilder::build()
{
	std::shared_ptr<GameObject> gameObject(new GameObject(_name));
	std::for_each(_children.begin(), _children.end(), [&](const std::shared_ptr<GameObject> &g)
								{ gameObject->addChildNode(g); });
	std::for_each(_components.begin(), _components.end(), [&gameObject](const std::shared_ptr<Component> &c)
								{ gameObject->addComponent(c); });
	Transform &transform = gameObject->transform()
														 .setPosition(_position)
														 .setScale(_scale)
														 .setRotation(_rotation);

	if (_target.has_value())
	{
		transform.lookAt(_position, *_target);
	}
	return gameObject;
}
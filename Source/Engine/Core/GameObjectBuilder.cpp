#include "GameObjectBuilder.h"

#include <algorithm>

#include "Component.h"
#include "GameObject.h"
#include "Scene.h"

GameObjectBuilder::GameObjectBuilder(Scene &scene) : _scene(scene), _scale(Vector3::Identity),
																										 _position(Vector3::Zero),
																										 _rotation(Quaternion::Identity)
{
}

GameObjectBuilder &GameObjectBuilder::withName(const std::string &name)
{
	_name = name;
	return *this;
}

GameObjectBuilder &GameObjectBuilder::withChild(GameObject &gameObject)
{
	_children.push_back(gameObject);
	return *this;
}

GameObjectBuilder &GameObjectBuilder::withComponent(Component &component)
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

GameObjectBuilder &GameObjectBuilder::withRotation(const Quaternion &rotation)
{
	_rotation = rotation;
	return *this;
}

GameObject &GameObjectBuilder::build()
{
	GameObject &gameObject = _scene.createGameObject(_name);
	std::for_each(_children.begin(), _children.end(), [&gameObject](std::reference_wrapper<GameObject> g)
								{ gameObject.addChild(g.get()); });
	std::for_each(_components.begin(), _components.end(), [&gameObject](std::reference_wrapper<Component> c)
								{ gameObject.addComponent(c.get()); });
	gameObject.transform()
			.setPosition(_position)
			.setScale(_scale)
			.setRotation(_rotation);
	return gameObject;
}
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "Maths.h"

class Component;
class GameObject;
class Scene;

class GameObjectBuilder
{
public:
	GameObjectBuilder(Scene &scene);

	GameObjectBuilder &withName(const std::string &name);
	GameObjectBuilder &withChild(GameObject &gameObject);
	GameObjectBuilder &withComponent(Component &component);
	GameObjectBuilder &withScale(const Vector3 &scale);
	GameObjectBuilder &withPosition(const Vector3 &position);
	GameObjectBuilder &withRotation(const Quaternion &rotation);

	GameObject &build();

private:
	std::vector<std::reference_wrapper<GameObject>> _children;
	std::vector<std::reference_wrapper<Component>> _components;
	std::string _name;
	Scene &_scene;
	Vector3 _scale;
	Vector3 _position;
	Quaternion _rotation;
};
#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Maths.h"

class Component;
class GameObject;
class Scene;

class GameObjectBuilder
{
public:
	GameObjectBuilder();

	GameObjectBuilder &withName(const std::string &name);
	GameObjectBuilder &withChild(const std::shared_ptr<GameObject> &childObject);
	GameObjectBuilder &withComponent(const std::shared_ptr<Component> &component);
	GameObjectBuilder &withScale(const Vector3 &scale);
	GameObjectBuilder &withPosition(const Vector3 &position);
	GameObjectBuilder &withTarget(const Vector3 &target);
	GameObjectBuilder &withRotation(const Quaternion &rotation);

	std::shared_ptr<GameObject> build();

private:
	std::vector<std::shared_ptr<GameObject>> _children;
	std::vector<std::shared_ptr<Component>> _components;
	std::string _name;
	Vector3 _scale;
	Vector3 _position;
	std::optional<Vector3> _target;
	Quaternion _rotation;
};
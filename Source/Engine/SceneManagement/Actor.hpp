#pragma once
#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include "../Maths/AABB.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"
#include "Component.hpp"

class SceneNode;
class Transform;

class Actor
{
public:
	Actor(const std::string& name);

	template <typename T>
	std::shared_ptr<T> CreateComponent();

	template <typename T>
	void AddComponent(const std::shared_ptr<Component>& component);

	void SetParent(const std::shared_ptr<SceneNode>& parent);
	void SetTransform(const std::shared_ptr<Transform>& transform);

	void SetPosition(const Vector3& position);
	void SetScale(const Vector3& scale);
	void SetRotation(const Quaternion& rotation);

	void Rotate(const Quaternion& rotationDelta);

	template <typename T>
	std::shared_ptr<T> GetComponent() const;

	std::shared_ptr<Transform> GetTransform() const;

	void CalculateBounds();
	Aabb GetBounds();

private:
	std::string _name;
	std::shared_ptr<SceneNode> _parent;
	std::shared_ptr<Transform> _transform;
	std::vector<std::shared_ptr<Component>> _components;

	bool _calculatedBounds;
	Aabb _bounds;
};

template <typename T>
std::shared_ptr<T> Actor::CreateComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "Template type must be a child class of Component");
	std::shared_ptr<T> component(new T());
	AddComponent<T>(component);
	return component;
}

template <typename T>
std::shared_ptr<T> Actor::GetComponent() const
{
	auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component>& c)
	{
		return std::dynamic_pointer_cast<T>(c) != nullptr;
	});

	return iter != _components.end() ? std::static_pointer_cast<T>(*iter) : nullptr;
}

template <typename T>
void Actor::AddComponent(const std::shared_ptr<Component>& component)
{
	static_assert(std::is_base_of<Component, T>::value, "Template type must be a child class of Component");
	auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component>& c)
	{
		return std::dynamic_pointer_cast<T>(c) != nullptr;
	});
	
	if (iter != _components.end())
	{
		*iter = component;
	}
	_components.push_back(component);
}
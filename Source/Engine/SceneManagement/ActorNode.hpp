#pragma once
#include "Component.hpp"
#include "SceneNode.hpp"
#include "Transform.h"

class ActorNode : public SceneNode
{
public:
	template <typename T>
	std::shared_ptr<T> CreateComponent();

	template <typename T>
	void AddComponent(const std::shared_ptr<Component>& component);

	template <typename T>
	std::shared_ptr<T> GetComponent() const;

	ActorNode();
	~ActorNode();

	std::string GetName() const
	{
		return _name;
	}

private:
	std::string _name;
	std::vector<std::shared_ptr<Component>> _components;
};

template <typename T>
std::shared_ptr<T> ActorNode::CreateComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "Template type must be a child class of Component");
	std::shared_ptr<T> component(new T());
	AddComponent<T>(component);
	return component;
}

template <typename T>
std::shared_ptr<T> ActorNode::GetComponent() const
{
	auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component>& c)
		{
			return std::dynamic_pointer_cast<T>(c) != nullptr;
		});

	return iter != _components.end() ? std::static_pointer_cast<T>(*iter) : nullptr;
}

template <typename T>
void ActorNode::AddComponent(const std::shared_ptr<Component>& component)
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

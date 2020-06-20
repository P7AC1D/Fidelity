#pragma once
#include <string>

#include "Component.hpp"
#include "SceneNode.hpp"

class ActorNode final : public SceneNode
{
	friend class SceneNode;
	
public:
	template <typename T>
	static sptr<T> CreateComponent();

	template <typename T>
	void AddComponent(const sptr<T>& component);

	template <typename T>
	std::shared_ptr<T> GetComponent() const;
	
	~ActorNode() = default;

	void OnDrawInspector() override;
	void OnDraw(std::shared_ptr<Renderer> renderer) override;
	void OnUpdate(float64 dt) override;

	SceneNodeType GetNodeType() const override;

protected:
	ActorNode() = default;

private:
	std::vector<sptr<Component>> _components;
};

template <typename T>
std::shared_ptr<T> ActorNode::CreateComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "Template type must be a child class of Component");
	
	std::shared_ptr<T> component(new T());
	return component;
}

template <typename T>
sptr<T> ActorNode::GetComponent() const
{
	auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component>& c)
		{
			return std::dynamic_pointer_cast<T>(c) != nullptr;
		});

	return iter != _components.end() ? std::static_pointer_cast<T>(*iter) : nullptr;
}

template <typename T>
void ActorNode::AddComponent(const sptr<T>& component)
{
	static_assert(std::is_base_of<Component, T>::value, "Template type must be a child class of Component");
	
	auto iter = std::find_if(_components.begin(), _components.end(), [](const std::shared_ptr<Component>& c)
		{
			return std::static_pointer_cast<Component>(c) != nullptr;
		});

	if (iter != _components.end())
	{
		*iter = component;
	}

	component->_parent = shared_from_this();
	_components.push_back(component);
}

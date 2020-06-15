#pragma once
#include <memory>

#include "SceneNode.hpp"

class Component
{
	friend class ActorNode;
	
public:
	Component() = default;
	Component(const Component& other) = default;
	Component(Component&& other) noexcept = default;
	Component& operator=(const Component& other) = default;
	Component& operator=(Component&& other) noexcept = default;
	virtual ~Component() = default;

	virtual void Draw(const std::shared_ptr<Renderer>& renderer) = 0;
	virtual void Update() = 0;

	sptr<SceneNode> GetParent() const { return _parent; }

protected:
	sptr<SceneNode> _parent;
};

typedef std::shared_ptr<Component> ComponentPtr;
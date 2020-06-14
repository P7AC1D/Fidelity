#pragma once
#include <memory>
#include <string>
#include <vector>

#include "ISceneNode.hpp"
#include "Transform.h"

class SceneNode : public ISceneNode
{
public:
	SceneNode() = default;
	SceneNode(const SceneNode& other) = default;
	SceneNode(SceneNode&& other) noexcept = default;
	SceneNode& operator=(const SceneNode& other) = default;
	SceneNode& operator=(SceneNode&& other) noexcept = default;
	virtual ~SceneNode() = default;

	virtual void OnUpdate(float64 dt) = 0;

	void Update(float64 dt) override
	{
		for (const auto& child : _childNodes)
		{
			child->Update(dt);
		}
		OnUpdate(dt);
	}
		
	void AddChildNode(SceneNodePtr childNode) override;

	const std::vector<SceneNodePtr>& GetAllChildNodes() const
	{
		return _childNodes;
	}

	Transform& GetTransform() override
	{
		return _transform;
	}

	void SetTransform(const Transform& transform) override
	{
		_transform = transform;
	}

private:
	Transform _transform;
	std::vector<SceneNodePtr> _childNodes;
};


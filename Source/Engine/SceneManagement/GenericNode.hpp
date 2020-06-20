#pragma once
#include "SceneNode.hpp"

class GenericNode final : public SceneNode
{
	friend class SceneNode;
	
public:
	void OnDrawInspector() override {}

	void OnDraw(std::shared_ptr<Renderer> renderer) override
	{
		
	}
	
	void OnUpdate(float64 dt) override
	{		
	}

	SceneNodeType GetNodeType() const override { return SceneNodeType::Generic; }

protected:
	GenericNode() = default;
};

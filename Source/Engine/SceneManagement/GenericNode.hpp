#pragma once
#include "SceneNode.hpp"

class GenericNode final : public SceneNode
{
public:
	void OnDraw(std::shared_ptr<Renderer> renderer) override
	{
		
	}
	
	void OnUpdate(float64 dt) override
	{		
	}
};

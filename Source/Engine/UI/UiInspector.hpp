#pragma once
#include <memory>
#include "../Core/Types.hpp"

class SceneNode;

class UiInspector
{
public:
	static void Build(const std::shared_ptr<SceneNode>& node);
};
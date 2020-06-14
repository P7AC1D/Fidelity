#pragma once
#include "ActorNode.hpp"
#include "GenericNode.hpp"
#include "SceneNode.hpp"

// TODO: change this to a SceneNodeBuilderFactory and create builder classes.
class SceneNodeFactory
{
public:
	static std::shared_ptr<GenericNode> CreateGeneric();

	static std::shared_ptr<ActorNode> CreateActor();

	//static std::shared_ptr<CameraNode> CreateCamera();

	//static std::shared_ptr<LightNode> CreateLight();
};

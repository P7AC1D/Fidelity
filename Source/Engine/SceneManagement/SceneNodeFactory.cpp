#include "SceneNodeFactory.hpp"

std::shared_ptr<GenericNode> SceneNodeFactory::CreateGeneric()
{
	return std::make_shared<GenericNode>();
}

std::shared_ptr<ActorNode> SceneNodeFactory::CreateActor()
{
	return std::make_shared<ActorNode>();
}

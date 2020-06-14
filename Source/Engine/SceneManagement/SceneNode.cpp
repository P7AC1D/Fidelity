#include "SceneNode.hpp"

#include "Actor.hpp"
#include "Transform.h"

void SceneNode::AddChildNode(SceneNodePtr childNode)
{
	_childNodes.push_back(childNode);
}
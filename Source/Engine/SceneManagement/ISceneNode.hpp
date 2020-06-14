#pragma once
#include <memory>

#include "../Core/Types.hpp"

class ISceneNode;
class Transform;

typedef std::shared_ptr<ISceneNode> SceneNodePtr;

class ISceneNode
{
public:
	virtual ~ISceneNode() = default;
	virtual void Update(float64 dt) = 0;
	virtual void AddChildNode(SceneNodePtr childNode) = 0;
	virtual Transform& GetTransform() = 0;
	virtual void SetTransform(const Transform& transform) = 0;
};

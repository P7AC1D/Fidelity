#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"

class Actor;
class Transform;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
	SceneNode(const std::string& name);

	std::shared_ptr<SceneNode> CreateChildNode(const std::string& name);
	std::shared_ptr<Actor> CreateActor(const std::string& name);

	void SetTransform(const std::shared_ptr<Transform>& transform);
	void SetParentNode(const std::shared_ptr<SceneNode>& parent);

	void AddChildNode(const std::shared_ptr<SceneNode>& child);
	void AddActor(const std::shared_ptr<Actor>& object);

	void SetPosition(const Vector3& position);
	void SetScale(const Vector3& scale);
	void SetRotation(const Quaternion& rotation);

	void Rotate(const Quaternion& rotationDelta);

	std::vector<std::shared_ptr<Actor>> GetActors() const;
	std::vector<std::shared_ptr<SceneNode>> GetChildNodes() const;
	std::vector<std::shared_ptr<Actor>> GetAllActors() const;

private:
	std::string _name;
	std::shared_ptr<Transform> _transform;
	std::shared_ptr<SceneNode> _parentNode;
	std::vector<std::shared_ptr<Actor>> _actors;
	std::vector<std::shared_ptr<SceneNode>> _childNodes;
};

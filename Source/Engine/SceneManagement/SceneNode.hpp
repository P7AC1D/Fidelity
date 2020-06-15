#pragma once
#include <memory>
#include <vector>

#include "Transform.h"

class Renderer;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
	template <typename T>
	static std::shared_ptr<T> Create();
	
	SceneNode() = default;
	SceneNode(const SceneNode& other) = default;
	SceneNode(SceneNode&& other) noexcept = default;
	SceneNode& operator=(const SceneNode& other) = default;
	SceneNode& operator=(SceneNode&& other) noexcept = default;
	virtual ~SceneNode() = default;

	void Draw(std::shared_ptr<Renderer> renderer);
	void Update(float64 dt);

	virtual void OnDraw(std::shared_ptr<Renderer> renderer) = 0;
	virtual void OnUpdate(float64 dt) = 0;

	template <typename T>
	void AddChild(const sptr<T>& node);

	const std::vector<sptr<SceneNode>>& GetAllChildNodes() const
	{
		return _childNodes;
	}

	Transform& GetTransform();
	void SetTransform(const Transform& transform);

private:
	std::vector<sptr<SceneNode>> _childNodes;
	Transform _transform;
};

template<typename T>
std::shared_ptr<T> SceneNode::Create()
{
	static_assert(std::is_base_of<SceneNode, T>::value, "Template type must be a child class of Component");
	return std::make_shared<T>();
}

template<typename T>
void SceneNode::AddChild(const sptr<T>& node)
{
	_childNodes.push_back(std::static_pointer_cast<SceneNode, T>(node));
}
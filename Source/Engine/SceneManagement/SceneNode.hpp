#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Transform.h"

class Renderer;

enum class SceneNodeType
{
	Generic,
	Actor,
	Camera,
	Light
};

inline std::string SceneNodeTypeToString(SceneNodeType sceneNodeType)
{
	switch (sceneNodeType)
	{
	default:
	case SceneNodeType::Generic: return "GenericNode";
	case SceneNodeType::Actor: return "ActorNode";
	case SceneNodeType::Camera: return "CameraNode";
	case SceneNodeType::Light: return "LightNode";
	}
}

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
	template <typename T>
	static std::shared_ptr<T> Create(const std::string& name = "");
	
	SceneNode() : _selectedInEditor(false) {}
	SceneNode(const SceneNode& other) = default;
	SceneNode(SceneNode&& other) noexcept = default;
	SceneNode& operator=(const SceneNode& other) = default;
	SceneNode& operator=(SceneNode&& other) noexcept = default;
	virtual ~SceneNode() = default;

	void Draw(std::shared_ptr<Renderer> renderer);
	void DrawInspector();
	void Update(float64 dt);

	virtual void OnDraw(std::shared_ptr<Renderer> renderer) = 0;
	virtual void OnDrawInspector() = 0;
	virtual void OnUpdate(float64 dt) = 0;
	
	virtual SceneNodeType GetNodeType() const = 0;

	template <typename T>
	void AddChild(const sptr<T>& node);

	std::string GetName() const;
	void SetName(const std::string& name);
	
	const std::vector<sptr<SceneNode>>& GetAllChildNodes() const
	{
		return _childNodes;
	}

	Transform& GetTransform();
	void SetTransform(const Transform& transform);

	const Transform& GetWorldTransform() const { return _worldTransform; }

	bool IsSelected() const { return _selectedInEditor; }
	void SetSelected(bool selected) { _selectedInEditor = selected; }
	
private:
	void UpdateTransform();
	
	static uint64 _id;
	
	bool _selectedInEditor;
	std::string _name;
	std::vector<sptr<SceneNode>> _childNodes;
	sptr<SceneNode> _parentNode;
	Transform _transform;
	Transform _worldTransform;
};

template<typename T>
std::shared_ptr<T> SceneNode::Create(const std::string& name)
{
	static_assert(std::is_base_of<SceneNode, T>::value, "Template type must be a child class of SceneNode");
	
	sptr<T> ptr(new T);
	if (!name.empty())
	{
		ptr->SetName(name);
	}
	else
	{
		ptr->SetName(SceneNodeTypeToString(ptr->GetNodeType()) + "_" + std::to_string(++_id));
	}
	return ptr;
}

template<typename T>
void SceneNode::AddChild(const sptr<T>& node)
{
	static_assert(std::is_base_of<SceneNode, T>::value, "Template type must be a child class of SceneNode");

	node->_parentNode = shared_from_this();
	_childNodes.push_back(std::static_pointer_cast<SceneNode, T>(node));
}
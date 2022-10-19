#include "GameObject.h"

#include "../Core/Maths.h"
#include "../UI/ImGui/imgui.h"
#include "../Rendering/Drawable.h"
#include "Component.h"

GameObject::GameObject() : _index(0)
{
}

GameObject::GameObject(const std::string &name, uint64 index) : _name(name), _index(index), _parent(nullptr)
{
}

void GameObject::update(float32 dt)
{
	if (_localTransform.modified())
	{
		if (_parent == nullptr)
		{
			_globalTransform = _localTransform;
		}
		else
		{
			_globalTransform = _parent->getLocalTransform() * _localTransform;
		}

		_localTransform.update(dt);
		updateChildNodeTransforms(dt);
		notifyComponents();
	}

	if (_globalTransform.modified())
	{
		_globalTransform.update(dt);
		notifyComponents();
	}
}

void GameObject::drawInspector()
{
	ImGui::Separator();
	ImGui::Text(("Name: " + _name).c_str());
	ImGui::Separator();

	ImGui::Text("Transform");
	{
		Vector3 position = _localTransform.getPosition();
		float32 pos[]{position.X, position.Y, position.Z};
		if (ImGui::DragFloat3("Position", pos, 0.1f))
		{
			_localTransform.translate(Vector3(position.X - pos[0], position.Y - pos[1], position.Z - pos[2]));
		}
	}
	{
		Vector3 scale = _localTransform.getScale();
		float32 scl[]{scale.X, scale.Y, scale.Z};
		if (ImGui::DragFloat3("Scale", scl, 0.001f))
		{
			_localTransform.scale(Vector3(scale.X - scl[0], scale.Y - scl[1], scale.Z - scl[2]));
		}
	}
	{
		auto euler = _localTransform.getRotation().ToEuler();
		float32 angles[3] = {euler[0].InDegrees(), euler[1].InDegrees(), euler[2].InDegrees()};
		if (ImGui::DragFloat3("Orientation", angles, 1.0f, -180.0f, 180.0f))
		{
			_localTransform.rotate(Quaternion(euler[0] - Degree(angles[0]), euler[1] - Degree(angles[1]), euler[2] - Degree(angles[2])));
		}
	}

	for (auto component : _components)
	{
		component->drawInspector();
	}
}

GameObject &GameObject::addComponent(Component &component)
{
	_components.push_back(&component);
	return *this;
}

GameObject &GameObject::addChildNode(GameObject &gameObject)
{
	gameObject._parent = this;
	_childNodes.push_back(&gameObject);
	return *this;
}

void GameObject::updateChildNodeTransforms(float32 dt)
{
	for (auto childNode : _childNodes)
	{
		childNode->_globalTransform = _localTransform * childNode->_localTransform;
	}
}

void GameObject::notifyComponents() const
{
	for (auto component : _components)
	{
		component->notify(*this);
	}
}
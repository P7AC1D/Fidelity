#include "GameObject.h"

#include "../Core/Maths.h"
#include "../UI/ImGui/imgui.h"
#include "../Rendering/Drawable.h"
#include "Component.h"

GameObject::GameObject() : _index(0)
{
}

GameObject::GameObject(const std::string &name, uint64 index) : _name(name), _index(index)
{
}

void GameObject::update(float32 dt)
{
	if (_transform.modified())
	{
		_transform.update(dt);
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
		Vector3 position = _transform.getPosition();
		float32 pos[]{position.X, position.Y, position.Z};
		if (ImGui::DragFloat3("Position", pos, 0.1f))
		{
			_transform.translate(Vector3(position.X - pos[0], position.Y - pos[1], position.Z - pos[2]));
		}
	}
	{
		Vector3 scale = _transform.getScale();
		float32 scl[]{scale.X, scale.Y, scale.Z};
		if (ImGui::DragFloat3("Scale", scl, 0.1f))
		{
			_transform.scale(Vector3(scale.X - scl[0], scale.Y - scl[1], scale.Z - scl[2]));
		}
	}
	{
		auto euler = _transform.getRotation().ToEuler();
		float32 angles[3] = {euler[0].InDegrees(), euler[1].InDegrees(), euler[2].InDegrees()};
		if (ImGui::DragFloat3("Orientation", angles, 1.0f, -180.0f, 180.0f))
		{
			_transform.rotate(Quaternion(euler[0] - Degree(angles[0]), euler[1] - Degree(angles[1]), euler[2] - Degree(angles[2])));
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

void GameObject::notifyComponents() const
{
	for (auto component : _components)
	{
		component->notify(*this);
	}
}
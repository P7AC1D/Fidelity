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
		ImGui::DragFloat3("Position", pos, 0.1f);
		_transform.setPosition(Vector3(pos[0], pos[1], pos[2]));
	}
	{
		Vector3 scale = _transform.getScale();
		float32 scl[]{scale.X, scale.Y, scale.Z};
		ImGui::DragFloat3("Scale", scl, 0.1f);
		_transform.setScale(Vector3(scl[0], scl[1], scl[2]));
	}
	{
		Vector3 euler = _transform.getRotation().ToEuler();
		float32 angles[3] = {euler.X, euler.Y, euler.Z};
		ImGui::DragFloat3("Orientation", angles, 1.0f);
		_transform.setRotation(Quaternion(Degree(angles[0]), Degree(angles[1]), Degree(angles[2])));
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
#include "SceneNode.hpp"

#include "../Maths/Degree.hpp"
#include "../Rendering/Renderer.h"
#include "../UI/ImGui/imgui.h"

uint64 SceneNode::_id = 0;

void SceneNode::Draw(std::shared_ptr<Renderer> renderer)
{
	for (const auto &child : _childNodes)
	{
		child->Draw(renderer);
	}
	OnDraw(renderer);
}

void SceneNode::DrawInspector()
{
	ImGui::Separator();
	ImGui::Text(_name.c_str());
	ImGui::Separator();

	ImGui::Text("Transform");
	{
		Vector3 position = _transform.GetPosition();
		float32 pos[]{position.X, position.Y, position.Z};
		ImGui::DragFloat3("Position", pos, 0.1f);
		_transform.SetPosition(Vector3(pos[0], pos[1], pos[2]));
	}

	{
		Vector3 scale = _transform.GetScale();
		float32 scl[]{scale.X, scale.Y, scale.Z};
		ImGui::DragFloat3("Scale", scl, 0.1f);
		_transform.SetScale(Vector3(scl[0], scl[1], scl[2]));
	}

	{
		Vector3 euler = _transform.GetRotation().ToEuler();
		float32 angles[3] = {euler.X, euler.Y, euler.Z};
		ImGui::DragFloat3("Orientation", angles, 1.0f);
		_transform.SetRotation(Quaternion(Degree(angles[0]), Degree(angles[1]), Degree(angles[2])));
	}

	OnDrawInspector();
}

void SceneNode::Update(float64 dt)
{
	UpdateTransform();
	for (const auto &child : _childNodes)
	{
		child->Update(dt);
	}
	OnUpdate(dt);
}

std::string SceneNode::GetName() const
{
	return _name;
}

void SceneNode::SetName(const std::string &name)
{
	_name = name;
}

Transform &SceneNode::GetTransform()
{
	return _transform;
}

void SceneNode::SetTransform(const Transform &transform)
{
	_transform = transform;
}

void SceneNode::UpdateTransform()
{
	_transform.Update();
	if (!_parentNode)
	{
		return;
	}

	Transform &parentTransform = _parentNode->GetTransform();
	if (parentTransform.Modified())
	{
		_worldTransform = Transform(parentTransform.GetMatrix() * _transform.GetMatrix());
	}
}

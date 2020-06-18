#include "ActorNode.hpp"

#include "../UI/ImGui/imgui.h"

void ActorNode::OnDrawInspector()
{
	ImGui::Separator();
	{		
		for (auto component : _components)
		{
			component->DrawInspector();
		}
	}
}

void ActorNode::OnDraw(std::shared_ptr<Renderer> renderer)
{
	for (const auto& component : _components)
	{
		component->Draw(renderer);
	}
}

void ActorNode::OnUpdate(float64 dt)
{
	for (const auto& component : _components)
	{
		component->Update();
	}	
}

SceneNodeType ActorNode::GetNodeType() const
{
	return SceneNodeType::Actor;
}
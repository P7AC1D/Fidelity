#include "UiInspector.hpp"

#include "../Maths/Colour.hpp"
#include "../Maths/Degree.hpp"
#include "../Rendering/Material.hpp"
#include "../Rendering/Renderable.hpp"
#include "../Rendering/StaticMesh.h"
#include "../RenderApi/Texture.hpp"
#include "../SceneManagement/Transform.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"

void UiInspector::Build(const std::shared_ptr<SceneNode>& node)
{
	if (!node)
	{
		return;
	}

	ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	ImGui::Begin("Inspector", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

	node->DrawInspector();

	ImGui::SetWindowPos(ImVec2(screenSize.x - ImGui::GetWindowWidth(), 0));
	ImGui::End();
}
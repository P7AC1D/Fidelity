#include "UiInspector.hpp"

#include "../Maths/Colour.hpp"
#include "../Maths/Degree.hpp"
#include "../Rendering/Material.hpp"
#include "../Rendering/Renderable.hpp"
#include "../Rendering/StaticMesh.h"
#include "../RenderApi/Texture.hpp"
#include "../SceneManagement/Actor.hpp"
#include "../SceneManagement/Transform.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"

std::unordered_map<uint64, std::shared_ptr<Texture>> UiInspector::_textureCache;

void UiInspector::Build(const std::shared_ptr<Actor>& actor)
{
	if (!actor)
	{
		return;
	}

	ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	ImGui::Begin("Inspector", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

	BuildTransform(actor->GetTransform());
	BuildRenderable(actor->GetComponent<Renderable>());

	ImGui::SetWindowPos(ImVec2(screenSize.x - ImGui::GetWindowWidth(), 0));
	ImGui::End();
}

void UiInspector::PushTextureToCache(uint64 ptr, const std::shared_ptr<Texture>& texture)
{
	_textureCache[ptr] = texture;
}

std::shared_ptr<Texture> UiInspector::GetTextureFromCache(uint64 ptr)
{
	auto iter = _textureCache.find(ptr);
	if (iter == _textureCache.end())
	{
		return nullptr;
	}
	return iter->second;
}

void UiInspector::ClearCache()
{
	_textureCache.clear();
}

void UiInspector::BuildTransform(const std::shared_ptr<Transform>& transform)
{
	ImGui::Separator();
	ImGui::Text("Transform");
	{
		Vector3 position = transform->GetPosition();
		float32 pos[]{ position.X, position.Y, position.Z };
		ImGui::DragFloat3("Position", pos, 0.1f);
		transform->SetPosition(Vector3(pos[0], pos[1], pos[2]));
	}

	{
		Vector3 scale = transform->GetScale();
		float32 scl[]{ scale.X, scale.Y, scale.Z };
		ImGui::DragFloat3("Scale", scl, 0.1f);
		transform->SetScale(Vector3(scl[0], scl[1], scl[2]));
	}

	{
		Vector3 euler = transform->GetRotation().ToEuler();
		float32 angles[3] = { euler.X, euler.Y, euler.Z };
		ImGui::DragFloat3("Orientation", angles, 1.0f);
		transform->SetRotation(Quaternion(Degree(angles[0]), Degree(angles[1]), Degree(angles[2])));
	}
}


void UiInspector::BuildRenderable(const std::shared_ptr<Renderable>& renderable)
{
	if (!renderable)
	{
		return;
	}

	auto material = renderable->GetMesh()->GetMaterial();

	ImGui::Separator();
	ImGui::Text("Renderable");
	{
		Colour ambient = material->GetAmbientColour();
		float32 col[3] = { ambient[0], ambient[1], ambient[2] };
		ImGui::ColorEdit3("Ambient", col);
		material->SetAmbientColour(Colour(static_cast<uint8>(col[0] * 255), static_cast<uint8>(col[1] * 255), static_cast<uint8>(col[2] * 255)));
	}
	{
		Colour diffuse = material->GetDiffuseColour();
		float32 col[3] = { diffuse[0], diffuse[1], diffuse[2] };
		ImGui::ColorEdit3("Diffuse", col);
		material->SetDiffuseColour(Colour(static_cast<uint8>(col[0] * 255), static_cast<uint8>(col[1] * 255), static_cast<uint8>(col[2] * 255)));
	}
	{
		Colour specular = material->GetSpecularColour();
		float32 col[3] = { specular[0], specular[1], specular[2] };
		ImGui::ColorEdit3("Specular", col);
		material->SetSpecularColour(Colour(static_cast<uint8>(col[0] * 255), static_cast<uint8>(col[1] * 255), static_cast<uint8>(col[2] * 255)));
	}
	{
		float32 exponent = material->GetSpecularExponent();
		ImGui::SliderFloat("Exponent", &exponent, 0.0f, 256.0f);
		material->SetSpecularExponent(exponent);
	}
	{
		const char* items[] = { "Diffuse", "Normal", "Specular" };
		static int currentItem = 0;
		ImGui::Combo("Texture", &currentItem, items, 3);

		switch (currentItem)
		{
		case 0:
		{
			auto diffuseTexture = material->GetDiffuseTexture();
			if (diffuseTexture)
			{
				auto width = diffuseTexture->GetWidth();
				auto height = diffuseTexture->GetHeight();
				auto ratio = width / height;

				PushTextureToCache(reinterpret_cast<uint64>(&diffuseTexture), diffuseTexture);
				ImGui::Image(&diffuseTexture, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.9f, ImGui::GetWindowContentRegionWidth() * 0.9f / ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			}
			break;
		}
		case 1:
		{
			auto normalTexture = material->GetNormalTexture();
			if (normalTexture)
			{
				auto width = normalTexture->GetWidth();
				auto height = normalTexture->GetHeight();
				auto ratio = width / height;

				PushTextureToCache(reinterpret_cast<uint64>(&normalTexture), normalTexture);
				ImGui::Image(&normalTexture, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.9f, ImGui::GetWindowContentRegionWidth() * 0.9f / ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			}
			break;
		}
		case 2:
		{
			auto specularTexture = material->GetSpecularTexture();
			if (specularTexture)
			{
				auto width = specularTexture->GetWidth();
				auto height = specularTexture->GetHeight();
				auto ratio = width / height;

				PushTextureToCache(reinterpret_cast<uint64>(&specularTexture), specularTexture);
				ImGui::Image(&specularTexture, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.9f, ImGui::GetWindowContentRegionWidth() * 0.9f / ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			}
			break;
		}
		}

		//ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
		//if (ImGui::IsItemHovered())
		//{
		//	ImGui::BeginTooltip();
		//	float region_sz = 32.0f;
		//	float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > my_tex_w - region_sz) region_x = my_tex_w - region_sz;
		//	float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > my_tex_h - region_sz) region_y = my_tex_h - region_sz;
		//	float zoom = 4.0f;
		//	ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
		//	ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
		//	ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
		//	ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
		//	ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
		//	ImGui::EndTooltip();
		//}
	}
}
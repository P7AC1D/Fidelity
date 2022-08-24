#include "Renderable.hpp"

#include "../RenderApi/GpuBuffer.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/Texture.hpp"
#include "../UI/UiManager.hpp"
#include "../UI/ImGui/imgui.h"
#include "Renderer.h"
#include "StaticMesh.h"
#include "Material.hpp"

Renderable::Renderable() : Component(),
													 _drawBoundingBox(false)
{
	GpuBufferDesc desc;
	desc.BufferType = BufferType::Constant;
	desc.BufferUsage = BufferUsage::Stream;
	desc.ByteCount = sizeof(PerObjectBufferData);
	_perObjectBuffer = Renderer::GetRenderDevice()->CreateGpuBuffer(desc);
}

void Renderable::DrawInspector()
{
	ImGui::Text("Renderable");
	ImGui::Separator();
	{
		std::shared_ptr<Material> material = GetMesh()->GetMaterial();
		{
			Colour ambient = material->GetAmbientColour();
			float32 rawCol[3] = {ambient[0], ambient[1], ambient[2]};
			ImGui::ColorEdit3("Ambient", rawCol);
			material->SetAmbientColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));
		}
		{
			Colour diffuse = material->GetDiffuseColour();
			float32 rawCol[3] = {diffuse[0], diffuse[1], diffuse[2]};
			ImGui::ColorEdit3("Diffuse", rawCol);
			material->SetDiffuseColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));
		}
		{
			Colour specular = material->GetSpecularColour();
			float32 rawCol[3] = {specular[0], specular[1], specular[2]};
			ImGui::ColorEdit3("Specular", rawCol);
			material->SetSpecularColour(Colour(rawCol[0] * 255, rawCol[1] * 255, rawCol[2] * 255));
		}
		{
			float32 specular = material->GetSpecularExponent();
			ImGui::DragFloat("Exponent", &specular, 1.0f, 0.0f, 1000.0f);
			material->SetSpecularExponent(specular);
		}
		{
			std::vector<const char *> debugRenderingItems = {"Diffuse", "Opacity", "Normal", "Specular"};
			static int debugRenderingCurrentItem = 0;

			ImGui::Combo("Texture", &debugRenderingCurrentItem, debugRenderingItems.data(), debugRenderingItems.size());
			if (debugRenderingCurrentItem == 0)
			{
				auto diffuseTexture = material->GetDiffuseTexture();
				if (diffuseTexture == nullptr)
				{
					return;
				}

				UiManager::AddTexture(reinterpret_cast<uint64>(&diffuseTexture), diffuseTexture);
				ImGui::Image(
						&diffuseTexture,
						ImVec2(200, 200),
						ImVec2(0.0f, 0.0f),
						ImVec2(1.0f, 1.0f),
						ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
						ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
			}
			else if (debugRenderingCurrentItem == 1)
			{
				auto opacityTexture = material->GetOpacityTexture();
				if (opacityTexture == nullptr)
				{
					return;
				}

				UiManager::AddTexture(reinterpret_cast<uint64>(&opacityTexture), opacityTexture);
				ImGui::Image(
						&opacityTexture,
						ImVec2(200, 200),
						ImVec2(0.0f, 0.0f),
						ImVec2(1.0f, 1.0f),
						ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
						ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
			}
			else if (debugRenderingCurrentItem == 2)
			{
				auto normalTexture = material->GetNormalTexture();
				if (normalTexture == nullptr)
				{
					return;
				}

				UiManager::AddTexture(reinterpret_cast<uint64>(&normalTexture), normalTexture);
				ImGui::Image(
						&normalTexture,
						ImVec2(200, 200),
						ImVec2(0.0f, 0.0f),
						ImVec2(1.0f, 1.0f),
						ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
						ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
			}
			else if (debugRenderingCurrentItem == 3)
			{
				auto specularTexture = material->GetSpecularTexture();
				if (specularTexture == nullptr)
				{
					return;
				}

				UiManager::AddTexture(reinterpret_cast<uint64>(&specularTexture), specularTexture);
				ImGui::Image(
						&specularTexture,
						ImVec2(200, 200),
						ImVec2(0.0f, 0.0f),
						ImVec2(1.0f, 1.0f),
						ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
						ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
			}
		}
	}
}

void Renderable::Draw(const std::shared_ptr<Renderer> &renderer)
{
	PerObjectBufferData perObjectBufferData;
	perObjectBufferData.Model = GetParent()->GetWorldTransform().GetMatrix();
	perObjectBufferData.ModelView = renderer->GetBoundCamera()->GetView() * perObjectBufferData.Model;
	perObjectBufferData.ModelViewProjection = renderer->GetBoundCamera()->GetProj() * perObjectBufferData.ModelView;
	_perObjectBuffer->WriteData(0, sizeof(PerObjectBufferData), &perObjectBufferData, AccessType::WriteOnlyDiscard);

	renderer->SubmitRenderable(std::dynamic_pointer_cast<Renderable>(shared_from_this()));
}

void Renderable::Update()
{
}

void Renderable::SetMesh(const std::shared_ptr<StaticMesh> &mesh)
{
	_mesh = mesh;

	Vector3 max, min;
	auto positionData = mesh->GetPositionVertexData();
	for (auto position : positionData)
	{
		max.X = std::fmaxf(max.X, position.X);
		max.Y = std::fmaxf(max.Y, position.Y);
		max.Z = std::fmaxf(max.Z, position.Z);

		min.X = std::fminf(min.X, position.X);
		min.Y = std::fminf(min.Y, position.Y);
		min.Z = std::fminf(min.Z, position.Z);
	}

	_boundingBox = Aabb(max, min);
}

std::shared_ptr<StaticMesh> Renderable::GetMesh() const
{
	return _mesh;
}

std::shared_ptr<GpuBuffer> Renderable::GetPerObjectBuffer() const
{
	return _perObjectBuffer;
}
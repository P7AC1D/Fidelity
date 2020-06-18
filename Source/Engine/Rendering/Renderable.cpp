#include "Renderable.hpp"

#include "../RenderApi/GpuBuffer.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "Renderer.h"
#include "StaticMesh.h"
#include "../UI/ImGui/imgui.h"
#include "Material.hpp"

Renderable::Renderable(): Component()
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
	ImGui::BeginChild("Renderable", ImVec2(0, ImGui::GetFontSize() * 20), true);
	{
		std::shared_ptr<Material> material = GetMesh()->GetMaterial();

		{
			Colour diffuse = material->GetDiffuseColour();
			float32 rawCol[3] = { diffuse[0], diffuse[1], diffuse[2] };
			ImGui::ColorEdit3("Diffuse", rawCol);
			material->SetDiffuseColour(Colour(
				static_cast<uint8>(rawCol[0] * 255),
				static_cast<uint8>(rawCol[1] * 255),
				static_cast<uint8>(rawCol[2] * 255)));
		}
	}
	ImGui::EndChild();
}

void Renderable::Draw(const std::shared_ptr<Renderer>& renderer)
{
	PerObjectBufferData perObjectBufferData;
	perObjectBufferData.Model = GetParent()->GetTransform().Get();
	perObjectBufferData.ModelView = renderer->GetBoundCamera()->GetView() * perObjectBufferData.Model;
	perObjectBufferData.ModelViewProjection = renderer->GetBoundCamera()->GetProj() * perObjectBufferData.ModelView;
	_perObjectBuffer->WriteData(0, sizeof(PerObjectBufferData), &perObjectBufferData, AccessType::WriteOnlyDiscard);
	
	renderer->SubmitRenderable(std::dynamic_pointer_cast<Renderable>(shared_from_this()));
}

void Renderable::Update()
{
}

void Renderable::SetMesh(const std::shared_ptr<StaticMesh>& mesh)
{
	_mesh = mesh;
}

std::shared_ptr<StaticMesh> Renderable::GetMesh() const
{
	return _mesh;
}

std::shared_ptr<GpuBuffer> Renderable::GetPerObjectBuffer() const
{
	return _perObjectBuffer;
}
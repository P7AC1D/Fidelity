#include "Renderable.hpp"

#include "../RenderApi/GpuBuffer.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "Renderer.h"
#include "StaticMesh.h"

Renderable::Renderable(): Component()
{
	GpuBufferDesc desc;
	desc.BufferType = BufferType::Constant;
	desc.BufferUsage = BufferUsage::Stream;
	desc.ByteCount = sizeof(PerObjectBufferData);
	_perObjectBuffer = Renderer::GetRenderDevice()->CreateGpuBuffer(desc);
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
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

void Renderable::Update()
{
}

void Renderable::UpdatePerObjectBuffer(const PerObjectBufferData& perObjectData)
{
	_perObjectBuffer->WriteData(0, sizeof(PerObjectBufferData), &perObjectData, AccessType::WriteOnlyDiscard);
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

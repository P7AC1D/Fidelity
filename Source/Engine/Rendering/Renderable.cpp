#include "Renderable.hpp"

#include <algorithm>
#include "../RenderApi/GpuBuffer.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "Renderer.h"
#include "StaticMesh.h"

Renderable::Renderable(): Component(), _rendererNotified(false), _boundsCalculated(false)
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

Aabb Renderable::GetBounds()
{
	if (!_boundsCalculated)
	{
		CalculateBounds();
		_boundsCalculated = true;
	}
	return _bounds;
}

void Renderable::CalculateBounds()
{
	auto meshPositionData = GetMesh()->GetPositionVertexData();
	if (meshPositionData.empty())
	{
		return;
	}

	Vector3 max;
	Vector3 min;
	for (const auto& position : meshPositionData)
	{
		max.X = std::max(max.X, position.X);
		max.Y = std::max(max.Y, position.Y);
		max.Z = std::max(max.Z, position.Z);

		min.X = std::min(min.X, position.X);
		min.Y = std::min(min.Y, position.Y);
		min.Z = std::min(min.Z, position.Z);
	}

	_bounds.PositiveBounds = max;
	_bounds.NegativeBounds = min;
}
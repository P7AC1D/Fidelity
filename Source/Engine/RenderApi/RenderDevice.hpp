#pragma once
#include "BlendState.hpp"
#include "DepthStencilState.hpp"
#include "PipelineState.hpp"
#include "RasterizerState.hpp"
#include "VertexLayout.hpp"

class RenderDevice
{
public:
  virtual std::shared_ptr<BlendState> CreateBlendState(const BlendStateDesc& desc) = 0;
  virtual std::shared_ptr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc& desc) = 0;
  virtual std::shared_ptr<PipelineState> CreatePipelineState(const PipelineStateDesc& desc) = 0;
  virtual std::shared_ptr<RasterizerState> CreateRasterizerState(const RasterizerStateDesc& desc) = 0;
  virtual std::shared_ptr<VertexLayout> CreateVertexLayout(const VertexLayoutDesc& desc) = 0;

  virtual void SetPipelineState(std::weak_ptr<PipelineState> pipelineState) = 0;

  virtual void Draw(uint32 vertexCount, uint32 vertexOffset) = 0;
  virtual void DrawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset) = 0;
};
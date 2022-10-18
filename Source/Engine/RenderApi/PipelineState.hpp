#pragma once
#include <memory>
#include "../Core/Types.hpp"
#include "../Image/ImageFormat.hpp"
#include "ShaderParams.hpp"

class BlendState;
class DepthStencilState;
class RasterizerState;
class Shader;
class VertexLayout;
enum class PixelFormat;

enum class PrimitiveTopology
{
  TriangleList,
  TriangleStrip,
  PointList,
  LineList,
  PatchList
};

struct PipelineStateDesc
{
  std::shared_ptr<Shader> VS;
  std::shared_ptr<Shader> FS;
  std::shared_ptr<Shader> HS;
  std::shared_ptr<Shader> DS;
  std::shared_ptr<Shader> GS;
  std::shared_ptr<BlendState> BlendState;
  std::shared_ptr<RasterizerState> RasterizerState;
  std::shared_ptr<DepthStencilState> DepthStencilState;
  std::shared_ptr<VertexLayout> VertexLayout;
  std::shared_ptr<ShaderParams> ShaderParams;
  PrimitiveTopology Topology = PrimitiveTopology::TriangleList;
};

class PipelineState
{
  friend class RenderDevice;

public:
  const std::shared_ptr<Shader> &getVS() const { return _desc.VS; }
  const std::shared_ptr<Shader> &getFS() const { return _desc.FS; }
  const std::shared_ptr<Shader> &getHS() const { return _desc.HS; }
  const std::shared_ptr<Shader> &getDS() const { return _desc.DS; }
  const std::shared_ptr<Shader> &getGS() const { return _desc.GS; }
  const std::shared_ptr<BlendState> &getBlendState() const { return _desc.BlendState; }
  const std::shared_ptr<RasterizerState> &getRasterizerState() const { return _desc.RasterizerState; }
  const std::shared_ptr<DepthStencilState> &getDepthStencilState() const { return _desc.DepthStencilState; }
  const std::shared_ptr<VertexLayout> &getVertexLayout() const { return _desc.VertexLayout; }
  const std::shared_ptr<ShaderParams> getShaderParams() const { return _desc.ShaderParams; }
  PrimitiveTopology getPrimitiveTopology() const { return _desc.Topology; }

protected:
  PipelineState(const PipelineStateDesc &desc) : _desc(desc) {}

protected:
  PipelineStateDesc _desc;
};

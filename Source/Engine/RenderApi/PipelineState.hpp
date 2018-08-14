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

enum class PrimitiveTopologyType
{
  Undefined,
  Point,
  Line,
  Triangle,
  Patch
};

struct PipelineStateDesc
{
  std::shared_ptr<Shader> VS;
  std::shared_ptr<Shader> PS;
  std::shared_ptr<Shader> HS;
  std::shared_ptr<Shader> DS;
  std::shared_ptr<Shader> GS;
  std::shared_ptr<BlendState> BlendState;
  std::shared_ptr<RasterizerState> RasterizerState;
  std::shared_ptr<DepthStencilState> DepthStencilState;
  std::shared_ptr<VertexLayout> VertexLayout;
  std::shared_ptr<ShaderParams> ShaderParams;
  PrimitiveTopologyType Topology = PrimitiveTopologyType::Triangle;
};

class PipelineState
{
  friend class RenderDevice;
  
public:
  const std::shared_ptr<Shader>& GetVS() const { return _desc.VS; }
  const std::shared_ptr<Shader>& GetPS() const { return _desc.PS; }
  const std::shared_ptr<Shader>& GetHS() const { return _desc.HS; }
  const std::shared_ptr<Shader>& GetDS() const { return _desc.DS; }
  const std::shared_ptr<Shader>& GetGS() const { return _desc.GS; }
  const std::shared_ptr<BlendState>& GetBlendState() const { return _desc.BlendState; }
  const std::shared_ptr<RasterizerState>& GetRasterizerState() const { return _desc.RasterizerState; }
  const std::shared_ptr<DepthStencilState>& GetDepthStencilState() const { return _desc.DepthStencilState; }
  const std::shared_ptr<VertexLayout>& GetVertexLayout() const { return _desc.VertexLayout; }
  const std::shared_ptr<ShaderParams> GetShaderParams() const { return _desc.ShaderParams; }
  PrimitiveTopologyType GetPrimitiveTopology() const { return _desc.Topology; }

protected:
  PipelineState(const PipelineStateDesc& desc): _desc(desc) {}
  
protected:
  PipelineStateDesc _desc;
};

#pragma once
#include <memory>
#include "../Core/Types.hpp"

class BlendState;
class DepthStencilState;
class DomainShader;
class GeometryShader;
class HullShader;
class PixelShader;
class RasterizerState;
class VertexLayout;
class VertexShader;
enum class PixelFormat;
enum class PrimitiveTopology;

struct MultisampleDesc
{
  uint32 Count;
  uint32 Quality;

  MultisampleDesc() : Count(1), Quality(0) {}
};

struct PipelineStateObjectDesc
{
  std::weak_ptr<VertexShader> VS;
  std::weak_ptr<PixelShader> PS;
  std::weak_ptr<HullShader> HS;
  std::weak_ptr<DomainShader> DS;
  std::weak_ptr<GeometryShader> GS;
  std::weak_ptr<BlendState> BlendState;
  std::weak_ptr<RasterizerState> RasterizerState;
  std::weak_ptr<DepthStencilState> DepthStencilState;
  std::weak_ptr<VertexLayout> VertexLayout;
  PrimitiveTopology Topology;
  PixelFormat RTFormats[8];
  PixelFormat DSFormat;
  MultisampleDesc MultisampleDesc;
  uint32 RTCount;
};

class PipelineStateObject
{
public:
  const PipelineStateObjectDesc& GetDesc() const { return _desc; }

private:
  PipelineStateObjectDesc _desc;
};
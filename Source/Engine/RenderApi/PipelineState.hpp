#pragma once
#include <memory>
#include "../Core/Types.hpp"

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

struct MultisampleDesc
{
  uint32 Count = 1;
  uint32 Quality = 0;
};

struct PipelineStateDesc
{
  std::weak_ptr<Shader> VS;
  std::weak_ptr<Shader> PS;
  std::weak_ptr<Shader> HS;
  std::weak_ptr<Shader> DS;
  std::weak_ptr<Shader> GS;
  std::weak_ptr<BlendState> BlendState;
  std::weak_ptr<RasterizerState> RasterizerState;
  std::weak_ptr<DepthStencilState> DepthStencilState;
  std::weak_ptr<VertexLayout> VertexLayout;
  PrimitiveTopologyType Topology = PrimitiveTopologyType::Triangle;
  PixelFormat RTFormats[8];
  PixelFormat DSFormat;
  MultisampleDesc MultisampleDesc;
  uint32 RTCount = 0;
};

class PipelineState
{
public:
  const PipelineStateDesc& GetDesc() const { return _desc; }

protected:
  PipelineStateDesc _desc;
};
#pragma once
#include "../Core/Types.hpp"

enum class DepthWriteMask
{
  Zero,
  All
};

enum class ComparisonFunc
{
  Never,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always
};

enum class StencilOp
{
  Keep,
  Zero,
  Replace,
  IncrSat,
  DescSat,
  Invert,
  Incr,
  Decr
};

struct DepthStencilOpDesc
{
  StencilOp StencilFailOp = StencilOp::Keep;
  StencilOp StencilDepthFailOp = StencilOp::Keep;
  StencilOp StencilPassOp = StencilOp::Keep;
  ComparisonFunc StencilFunc = ComparisonFunc::Never;
};

struct DepthStencilStateDesc
{
  bool DepthEnabled = false;
  ComparisonFunc DepthFunc = ComparisonFunc::Never;
  DepthWriteMask DepthWriteMask = DepthWriteMask::Zero;
  bool StencilEnabled = false;
  byte StencilReadMask = 0xFF;
  byte StencilWriteMask = 0xFF;
  DepthStencilOpDesc FrontFace;
  DepthStencilOpDesc BackFace;
};

class DepthStencilState
{
public:
  const DepthStencilStateDesc& GetDesc() { return _desc; }

private:
  DepthStencilStateDesc _desc;
};
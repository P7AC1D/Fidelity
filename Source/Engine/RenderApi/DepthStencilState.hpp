#pragma once
#include "../Core/Types.hpp"

enum class DepthWriteMask
{
  Zero,
  All
};

enum class ComparisonFunction
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

enum class StencilOperation
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

struct StencilOperationDesc
{
  StencilOperation FailOp = StencilOperation::Keep;
  StencilOperation ZFailOp = StencilOperation::Keep;
  StencilOperation PassOp = StencilOperation::Keep;
  ComparisonFunction ComparisonFunc = ComparisonFunction::Never;
};

struct DepthStencilStateDesc
{
  bool DepthWriteEnabled = true;
  bool DepthReadEnabled = true;
  ComparisonFunction DepthFunc = ComparisonFunction::Less;
  bool StencilEnabled = false;
  byte StencilReadMask = 0xFF;
  byte StencilWriteMask = 0xFF;
  StencilOperationDesc FrontFace;
  StencilOperationDesc BackFace;
};

class DepthStencilState
{
  friend class RenderDevice;

public:
  const DepthStencilStateDesc &getDesc() { return _desc; }

protected:
  DepthStencilState(const DepthStencilStateDesc &desc) : _desc(desc) {}

protected:
  DepthStencilStateDesc _desc;
};

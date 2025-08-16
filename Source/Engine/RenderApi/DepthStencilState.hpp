#pragma once
#include "../Core/Types.hpp"

/**
 * @brief Controls whether depth writes are enabled.
 */
enum class DepthWriteMask
{
  Zero,
  All
};

/**
 * @brief Comparison functions used for depth and stencil tests.
 */
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

/**
 * @brief Stencil buffer operations when tests pass or fail.
 */
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

/**
 * @brief Stencil operations and function per face.
 */
struct StencilOperationDesc
{
  StencilOperation FailOp = StencilOperation::Keep;
  StencilOperation ZFailOp = StencilOperation::Keep;
  StencilOperation PassOp = StencilOperation::Keep;
  ComparisonFunction ComparisonFunc = ComparisonFunction::Never;
};

/**
 * @brief Depth and stencil testing configuration.
 */
struct DepthStencilStateDesc
{
  bool DepthWriteEnabled = true;
  bool DepthReadEnabled = true;
  ComparisonFunction DepthFunc = ComparisonFunction::Less;
  bool StencilEnabled = false;
  byte StencilReadMask = static_cast<byte>(0xFFu);
  byte StencilWriteMask = static_cast<byte>(0xFFu);
  StencilOperationDesc FrontFace;
  StencilOperationDesc BackFace;
};

/**
 * @brief Immutable depth-stencil state object.
 */
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

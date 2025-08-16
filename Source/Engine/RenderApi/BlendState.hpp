#pragma once
#include "../Core/Types.hpp"

/**
 * @brief Blend factors used when combining source and destination colors.
 */
enum class BlendFactor
{
  Zero,
  One,
  SrcColour,
  InvSrcColour,
  SrcAlpha,
  InvSrcAlpha,
  DestAlpha,
  InvDestAlpha,
  DestColour,
  InvDestColour
};

/**
 * @brief Blend operations used to combine source and destination terms.
 */
enum class BlendOperation
{
  Add,
  Subtract,
  RevSubtract,
  Min,
  Max
};

/**
 * @brief Color channel write mask.
 */
enum ColourWrite
{
  COLOUR_WRITE_DISABLE = 0,
  COLOUR_WRITE_ENABLE_RED = 1,
  COLOUR_WRITE_ENABLE_GREEN = 2,
  COLOUR_WRITE_ENABLE_BLUE = 4,
  COLOUR_WRITE_ENABLE_ALPHA = 8,
  COLOUR_WRITE_ENABLE_ALL = (((COLOUR_WRITE_ENABLE_RED | COLOUR_WRITE_ENABLE_GREEN) | COLOUR_WRITE_ENABLE_BLUE) | COLOUR_WRITE_ENABLE_ALPHA)
};

/**
 * @brief Per-channel blend function description.
 */
struct BlendDesc
{
  BlendFactor Source;
  BlendFactor Destination;
  BlendOperation Operation;

  BlendDesc(BlendFactor src, BlendFactor dst, BlendOperation op) : Source(src), Destination(dst), Operation(op) {}
};

/**
 * @brief Render target blend state configuration.
 */
struct RTBlendStateDesc
{
  bool BlendEnabled = false;
  BlendDesc Blend = BlendDesc(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha, BlendOperation::Add);
  BlendDesc BlendAlpha = BlendDesc(BlendFactor::One, BlendFactor::One, BlendOperation::Add);
  byte RTWriteMask = COLOUR_WRITE_ENABLE_ALL;
};

/**
 * @brief Global blend state configuration for a pipeline.
 */
struct BlendStateDesc
{
  bool AlphaToCoverageEnable = false;
  bool IndependentBlendEnable = false;
  RTBlendStateDesc RTBlendState[8];
};

/**
 * @brief Immutable blend state object.
 */
class BlendState
{
  friend class RenderDevice;

public:
  const BlendStateDesc &getDesc() const { return _desc; }

protected:
  BlendState(const BlendStateDesc &desc) : _desc(desc) {}

protected:
  BlendStateDesc _desc;
};

#pragma once
#include "../Core/Types.hpp"

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

enum class BlendOperation
{
  Add,
  Subtract,
  RevSubtract,
  Min,
  Max
};

enum ColourWrite
{
  COLOUR_WRITE_DISABLE = 0,
  COLOUR_WRITE_ENABLE_RED = 1,
  COLOUR_WRITE_ENABLE_GREEN = 2,
  COLOUR_WRITE_ENABLE_BLUE = 4,
  COLOUR_WRITE_ENABLE_ALPHA = 8,
  COLOUR_WRITE_ENABLE_ALL = (((COLOUR_WRITE_ENABLE_RED | COLOUR_WRITE_ENABLE_GREEN) | COLOUR_WRITE_ENABLE_BLUE) | COLOUR_WRITE_ENABLE_ALPHA)
};

struct BlendDesc
{
  BlendFactor Source;
  BlendFactor Destination;
  BlendOperation Operation;
  
  BlendDesc(BlendFactor src, BlendFactor dst, BlendOperation op): Source(src), Destination(dst), Operation(op) {}
};

struct RTBlendStateDesc
{
  bool BlendEnabled = false;
  BlendDesc Blend = BlendDesc(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha, BlendOperation::Add);
  BlendDesc BlendAlpha = BlendDesc(BlendFactor::One, BlendFactor::One, BlendOperation::Add);
  byte RTWriteMask = COLOUR_WRITE_ENABLE_ALL;
};

struct BlendStateDesc
{
  bool AlphaToCoverageEnable = false;
  bool IndependentBlendEnable = false;
  RTBlendStateDesc RTBlendState[8];
};

class BlendState
{
  friend class RenderDevice;
  
public:
  const BlendStateDesc& GetDesc() const { return _desc; }
  
protected:
  BlendState(const BlendStateDesc& desc): _desc(desc) {}

protected:
  BlendStateDesc _desc;
};

#pragma once
#include "../Core/Types.hpp"

enum class Blend
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
  InvDestColour,
  SrcAlphaSat,
  BlendFactor,
  InvBlendFactor,
  Src1Colour,
  InvSrc1Colour,
  Src1Alpha,
  InvSrc1Alpha
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

struct RTBlendStateDesc
{
  bool BlendEnabled = false;
  Blend SrcBlend = Blend::SrcAlpha;
  Blend DestBlend = Blend::InvSrcAlpha;
  BlendOperation BlendOp = BlendOperation::Add;
  Blend SrcBlendAlpha = Blend::One;
  Blend DestBlendAlpha = Blend::One;
  BlendOperation BlendOpAlpha = BlendOperation::Add;
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
public:
  const BlendStateDesc& GetDesc() const { return _desc; }

protected:
  BlendStateDesc _desc;
};
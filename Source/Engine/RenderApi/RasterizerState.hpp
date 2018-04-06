#pragma once
#include "../Core/Types.hpp"

enum class FillMode
{
  WireFrame,
  Solid
};

enum class CullMode
{
  None,
  Front,
  Back
};

struct RasterizerStateDesc
{
  FillMode FillMode = FillMode::Solid;
  CullMode CullMode = CullMode::None;
  bool FrontCounterClockwise = true;
  int32 DepthBias = 0;
  float32 DepthBiasClamp = 0.0f;
  float32 SlopeScaledDepthBias = 0.0f;
  bool DepthClipEnabled = false;
  bool MultisampleEnabled = false;
  bool AntialiasedLineEnable = false;
  uint32 ForcedSampleCount = 0;
  bool ConservativeRasterEnabled = false;
};

class RasterizerState
{
public: 
  const RasterizerStateDesc& GetDesc() const { return _desc; }

private:
  RasterizerStateDesc _desc;
};
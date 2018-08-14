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
  Clockwise,
  CounterClockwise
};

struct RasterizerStateDesc
{
  FillMode FillMode = FillMode::Solid;
  CullMode CullMode = CullMode::None;
  int32 DepthBias = 0;
  float32 DepthBiasClamp = 0.0f;
  float32 SlopeScaledDepthBias = 0.0f;
  bool DepthClipEnabled = false;
  bool ScissorEnabled = false;
  bool MultisampleEnabled = false;
  bool AntialiasedLineEnable = false;
};

class RasterizerState
{
  friend class RenderDevice;
  
public: 
	bool IsScissorEnabled() const { return _desc.ScissorEnabled; }
  const RasterizerStateDesc& GetDesc() const { return _desc; }
  
protected:
  RasterizerState(const RasterizerStateDesc& desc) : _desc(desc) {}

protected:
  RasterizerStateDesc _desc;
};

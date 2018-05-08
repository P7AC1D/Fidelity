#pragma once
#include <memory>
#include "Texture.hpp"

static const uint32 MaxColourTargets = 8;

struct RenderTargetDesc
{
  uint32 Width;
  uint32 Height;
  std::shared_ptr<Texture> ColourTargets[MaxColourTargets];
  std::shared_ptr<Texture> DepthStencilTarget;
};

class RenderTarget
{
public:
  const RenderTargetDesc& GetDesc() const { return _desc; }
  bool IsInitialized() const { return _isInitialized; }
  
protected:
  RenderTarget(const RenderTargetDesc& desc) : _desc(desc), _isInitialized(false) {}
  
protected:
  RenderTargetDesc _desc;
  bool _isInitialized;
};

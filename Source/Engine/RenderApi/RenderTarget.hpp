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
	std::shared_ptr<Texture> GetColourTarget(uint32 i) const { return i < MaxColourTargets ? _desc.ColourTargets[i] : nullptr; }
	std::shared_ptr<Texture> GetDepthStencilTarget() const { return _desc.DepthStencilTarget; }
  
protected:
  RenderTarget(const RenderTargetDesc& desc) : _desc(desc), _isInitialized(false) {}
  
protected:
  RenderTargetDesc _desc;
  bool _isInitialized;
};

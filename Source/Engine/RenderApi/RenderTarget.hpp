#pragma once
#include <memory>
#include "Texture.hpp"

static const uint32 MaxColourTargets = 8;

// Deprecated legacy render target description. Use FramebufferDesc instead.
struct [[deprecated("RenderTargetDesc is deprecated. Build FramebufferDesc with attachments instead.")]] RenderTargetDesc
{
  uint32 Width;
  uint32 Height;
  std::shared_ptr<Texture> ColourTargets[MaxColourTargets];
  std::shared_ptr<Texture> DepthStencilTarget;
};

/**
 * Deprecated legacy RenderTarget abstraction.
 * Use Framebuffer + FramebufferDesc and RenderPassBeginInfo.
 */
class [[deprecated("RenderTarget is deprecated. Replace with Framebuffer ownership and FramebufferDesc.")]] RenderTarget
{
public:
  virtual void copy(const std::shared_ptr<RenderTarget> &target) = 0;

  const RenderTargetDesc &getDesc() const { return _desc; }
  bool isInitialized() const { return _isInitialized; }
  std::shared_ptr<Texture> getColourTarget(uint32 i) const { return i < MaxColourTargets ? _desc.ColourTargets[i] : nullptr; }
  std::shared_ptr<Texture> getDepthStencilTarget() const { return _desc.DepthStencilTarget; }

protected:
  RenderTarget(const RenderTargetDesc &desc) : _desc(desc), _isInitialized(false) {}

protected:
  RenderTargetDesc _desc;
  bool _isInitialized;
};

#pragma once
#include <array>
#include <memory>

#include "../Core/Types.hpp"

namespace Rendering {

  class Texture;

  static const uint32 MaxColourBuffers = 4;

  struct RenderTargetDesc
  {
    uint32 Width;
    uint32 Height;
    uint32 ColourBufferCount;
    bool EnableStencilBuffer;
  };

  class RenderTarget
  {
  public:
    RenderTarget(RenderTargetDesc desc);
    ~RenderTarget();

    inline const RenderTargetDesc& GetDesc() const { return _desc; }
    inline std::shared_ptr<Texture> GetDepthStencilBuffer() const { return _depthStencilBuffer; }

    std::shared_ptr<Texture> GetColourBuffer(uint32 index) const;

    void Enable() const;
    void Disable() const;

  private:
    void Initialize();

  private:
    std::array<std::shared_ptr<Texture>, MaxColourBuffers> _colourBuffers;
    std::shared_ptr<Texture> _depthStencilBuffer;
    RenderTargetDesc _desc;
    uint32 _fbo;
  };
}
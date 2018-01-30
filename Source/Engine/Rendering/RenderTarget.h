#pragma once
#include "../Core/Types.hpp"

namespace Rendering {

  class Texture;

  struct RenderTargetDesc
  {
    uint32 width;
    uint32 height;
  };

  class RenderTarget
  {
  public:
    RenderTarget(RenderTargetDesc desc);

    void AttachColourBuffer(uint32 index, const Texture& colourBuffer);
    void AttachDepthBuffer(const Texture& depthBuffer);
  };
}
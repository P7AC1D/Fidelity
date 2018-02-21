#pragma once
#include "../Core/Types.hpp"

class Colour;

namespace Rendering {
  class ConstantBuffer;
  class IndexBuffer;
  class RasterizerState;
  class RenderTarget;
  class Shader;
  class Texture;
  class VertexBuffer;
  
  enum RenderTargetClearType : uint8
  {
    RTCT_Colour = 1 << 0,
    RTCT_Depth = 1 << 1,
  };
  
  class RenderApi
  {
  public:
    static void SetClearColour(const Colour& colour);
    static void SetConstantBuffer(uint32 slot, const ConstantBuffer& constantBuffer);
    static void SetIndexBuffer(const IndexBuffer& indexBuffer);
    static void SetRasterizerState(const RasterizerState& rasterizerState);
    static void SetRenderTarget(const RenderTarget& renderTarget);
    static void SetShader(const Shader& shader);
    static void SetTexture(uint32 slot, const Texture& texture);
    static void SetVertexBuffer(const VertexBuffer& vertexBuffer);
    static void SetViewport(int32 renderWidth, int32 renderHeight);
    
    static void ClearRenderTarget(uint8 target);

    static void Draw(uint32 offset, uint32 count);
  };
}

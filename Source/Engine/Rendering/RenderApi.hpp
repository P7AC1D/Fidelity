#pragma once
#include "../Core/Types.hpp"

class Colour;

namespace Rendering {
  class Shader;
  class Texture;
  class VertexBuffer;
  
  enum class ClearType
  {
    Color,
    Depth,
    All,
  };
  
  class RenderApi
  {
  public:
    static void SetClearColour(const Colour& colour);
    static void SetShader(const Shader& shader);
    static void SetTexture(uint32 target, const Texture& texture);
    static void SetVertexBuffer(const VertexBuffer& vertexBuffer);
    static void SetViewport(int32 renderWidth, int32 renderHeight);
    
    static void ClearBuffer(ClearType clearType);
  };
}

#pragma once
#include <string>

#include "../Core/Types.hpp"
#include "TextureCommon.hpp"

namespace Rendering 
{
struct TextureDesc
{
  uint32 Height;
  uint32 Width;
  PixelFormat Format;
  MinFilter MinFilter = MinFilter::Nearest;
  MagFilter MagFilter = MagFilter::Nearest;
  WrapMethod WrapMethod = WrapMethod::ClampToEdge;
};

class Texture
{
public:
  Texture(TextureDesc desc);
  Texture(TextureDesc desc, ubyte* data);
  ~Texture();

  inline const TextureDesc& GetDesc() const { return _desc; }

  void SetWrapMethod(WrapMethod wrapMethod);
  void SetMinFilter(MinFilter minFilter);
  void SetMagFilter(MagFilter magFilter);

  void GenerateMipMaps();
  void BindToTextureSlot(uint32 slot);
  
private:
  void Intialize(ubyte* data);

  void Bind();
  void Unbind();

private:
  uint32 _id;
  TextureDesc _desc;

  friend class RenderTarget;
  friend class Renderer;
};
}

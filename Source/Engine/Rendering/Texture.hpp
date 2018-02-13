#pragma once
#include <string>

#include "../Core/Types.hpp"

namespace Rendering {
enum class PixelFormat
{
  /// 8-bit red channel as unsigned bytes.
  R8,
  /// 8-bit red and green channels stored as unsigned bytes.
  RG8,
  /// 8-bit red, green and blue channels stored as unsigned bytes.
  RGB8,
  /// 8-bit red, green, blue and alpha channels stored as unsigned bytes.
  RGBA8,
  /// 16-bit red, green and blue channels stored as a signed floats.
  RGB16F,
  /// 16-bit red, green, blue and alpha channels stored as a signed floats.
  RGBA16F,
  /// 32-bit depth channel stored as unsigned bytes.
  D32,
  /// 24-bit depth channel with an 8 bit stencil channel stored as unsigned bytes.
  D24S8
};

enum class MinFilter
{
  Nearest,
  Linear,
  NearestMipmapNearest,
  NearestMipmapLinear,
  LinearMipmapNearest,
  LinearMipmapLinear
};

enum class MagFilter
{
  Nearest,
  Linear
};

enum class WrapMethod
{
  ClampToEdge,
  ClampToBorder,
  MirrorRepeat,
  Repeat
};

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

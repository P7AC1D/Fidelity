#pragma once
#include <string>

#include "../Core/Types.hpp"

namespace Rendering
{
enum class PixelFormat
{
  R8,
  RGB16F,
  RGB,
  RGBA,
  SRGB,
  SRGBA,
  Depth
};

enum class TextureMinFilter
{
  Nearest,
  Linear,
  NearestMipmapNearest,
  NearestMipmapLinear,
  LinearMipmapNearest,
  LinearMipmapLinear
};

enum class TextureMagFilter
{
  Nearest,
  Linear
};

enum class TextureWrapMethod
{
  ClampToEdge,
  ClampToBorder,
  MirrorRepeat,
  Repeat
};

class Texture
{
public:
  Texture(PixelFormat format, uint32 width, uint32 height);
  Texture(PixelFormat format, uint32 width, uint32 height, ubyte* data);
  ~Texture();

  void SetMinFilter(TextureMinFilter filter);
  void SetMagFilter(TextureMagFilter filter);
  void SetWrapMethod(TextureWrapMethod method);

  uint32 GetWidth() const { return _width; }
  uint32 GetHeight() const { return _height; }

  void BindToTextureSlot(uint32 slot);

private:
  void Bind() const;
  void Unbind() const;
  bool IsBound() const;

private:
  uint32 _id;
  PixelFormat _format;
  uint32 _width;
  uint32 _height;

  friend class FrameBuffer;
  friend class Renderer;
};
}

#pragma once
#include <string>

#include "../Core/Types.hpp"
#include "OpenGL.h"

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
  Nearest = GL_NEAREST,
  Linear = GL_LINEAR,
  NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
  NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
  LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
  LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
};

enum class TextureMagFilter
{
  Nearest = GL_NEAREST,
  Linear = GL_LINEAR
};

enum class TextureWrapMethod
{
  ClampToEdge = GL_CLAMP_TO_EDGE,
  ClampToBorder = GL_CLAMP_TO_BORDER,
  MirrorRepeat = GL_MIRRORED_REPEAT,
  Repeat = GL_REPEAT
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

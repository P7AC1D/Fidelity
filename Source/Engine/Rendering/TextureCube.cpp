#include "TextureCube.hpp"

#include <stdexcept>

#include "OpenGL.h"

namespace Rendering
{
static uint32 ActiveTexture = -1;

GLenum GetInternalFormat(PixelFormat format)
{
  switch (format)
  {
    case PixelFormat::R8: return GL_RED;
    case PixelFormat::RG8: return GL_RG;
    case PixelFormat::RGB8: return GL_RGB;
    case PixelFormat::RGBA8: return GL_RGBA;
    default: throw std::runtime_error("Unsupported PixelFormat for TextureCube.");
  }
}

GLenum GetWrapMethod(WrapMethod wrapMethod)
{
  switch (wrapMethod)
  {
    case WrapMethod::ClampToEdge: return GL_CLAMP_TO_EDGE;
    case WrapMethod::ClampToBorder: return GL_CLAMP_TO_BORDER;
    case WrapMethod::MirrorRepeat: return GL_MIRRORED_REPEAT;
    case WrapMethod::Repeat: return GL_REPEAT;
    default: return GL_CLAMP_TO_BORDER;
  }
}

GLenum GetMagFilter(MagFilter magFilter)
{
  switch (magFilter)
  {
    case MagFilter::Nearest: return GL_NEAREST;
    case MagFilter::Linear: return GL_LINEAR;
    default: return GL_NEAREST;
  }
}

GLenum GetMinFilter(MinFilter minFilter)
{
  switch (minFilter)
  {
    case MinFilter::Nearest: return GL_NEAREST;
    case MinFilter::Linear: return GL_LINEAR;
    case MinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
    case MinFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
    case MinFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
    case MinFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
    default: return GL_NEAREST;
  }
}

TextureCube::TextureCube(TextureCubeDesc desc):
  _desc(desc)
{
  GLCall(glGenTextures(1, &_id));
  Bind();

  SetWrapMethod(_desc.WrapMethod);
  SetMinFilter(_desc.MinFilter);
  SetMagFilter(_desc.MagFiler);
}

TextureCube::~TextureCube()
{
  Unbind();
  GLCall(glDeleteTextures(1, &_id));
}

void TextureCube::UploadData(TextureCubeFace face, const ubyte* data, uint32 level)
{
  Bind();
  GLenum format = GetInternalFormat(_desc.Format);
  GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLuint>(face), level, format, _desc.Width, _desc.Height, 0, format, GL_UNSIGNED_BYTE, data));
}

void TextureCube::SetWrapMethod(WrapMethod wrapMethod)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GetWrapMethod(wrapMethod)));
  GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GetWrapMethod(wrapMethod)));
  GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GetWrapMethod(wrapMethod)));
}

void TextureCube::SetMinFilter(MinFilter minFilter)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GetMinFilter(minFilter)));
}

void TextureCube::SetMagFilter(MagFilter magFilter)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GetMagFilter(magFilter)));
}

void TextureCube::GenerateMipMaps()
{
  Bind();
  GLCall(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
}

void TextureCube::BindToTextureSlot(uint32 slot)
{
  GLCall(glActiveTexture(GL_TEXTURE0 + slot));
  Bind();
}

void TextureCube::Bind()
{
  if (ActiveTexture != _id)
  {
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, _id));
    ActiveTexture = _id;
  }
}

void TextureCube::Unbind()
{
  if (ActiveTexture == _id)
  {
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    ActiveTexture = -1;
  }
}
}
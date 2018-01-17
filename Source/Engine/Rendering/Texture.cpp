#include "Texture.h"

namespace Rendering
{
static uint32 ActiveTexture = -1;

Texture::Texture(TextureFormat format, uint32 width, uint32 height) :
  Texture(format, width, height, nullptr)
{  
}

Texture::Texture(TextureFormat format, uint32 width, uint32 height, void* data) :
  _format(format),
  _width(width),
  _height(height)
{
  GLCall(glGenTextures(1, &_id));
  Bind();
  switch (format)
  {
    case TextureFormat::Red:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data));
      break;
    case TextureFormat::RGB:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
      break;
    case TextureFormat::RGBA:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
      break;
    case TextureFormat::SRGB:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
      break;
    case TextureFormat::SRGBA:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
      break;
    case TextureFormat::Depth:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data));
  }
  Unbind();
}

Texture::~Texture()
{
  Unbind();
  GLCall(glDeleteTextures(1, &_id));
}

void Texture::SetMinFilter(TextureMinFilter filter)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int32>(filter)));
}

void Texture::SetMagFilter(TextureMagFilter filter)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int32>(filter)));
}

void Texture::SetWrapMethod(TextureWrapMethod method)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int32>(method)));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int32>(method)));
}

void Texture::Bind() const
{
  if (!IsBound())
  {    
    GLCall(glBindTexture(GL_TEXTURE_2D, _id));
    ActiveTexture = _id;
  }
}

void Texture::Unbind() const
{
  if (IsBound())
  {
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    ActiveTexture = 0;
  }
}

bool Texture::IsBound() const
{
  return ActiveTexture == _id;
}
}
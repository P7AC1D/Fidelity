#include "Texture.h"

namespace Rendering
{
static uint32 ActiveTexture = -1;

Texture::Texture(PixelFormat format, uint32 width, uint32 height) :
  Texture(format, width, height, nullptr)
{  
}

Texture::Texture(PixelFormat format, uint32 width, uint32 height, ubyte* data) :
  _format(format),
  _width(width),
  _height(height)
{
  GLCall(glGenTextures(1, &_id));
  Bind();
  switch (format)
  {
    case PixelFormat::R8:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data));
      break;
    case PixelFormat::RGB16F:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data));
      break;
    case PixelFormat::RGB:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
      break;
    case PixelFormat::RGBA:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
      break;
    case PixelFormat::SRGB:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
      break;
    case PixelFormat::SRGBA:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
      break;
    case PixelFormat::Depth:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data));
  }
  SetMinFilter(TextureMinFilter::Nearest);
  SetMagFilter(TextureMagFilter::Nearest);
  //SetWrapMethod(TextureWrapMethod::Repeat);
  
  //Unbind();
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

void Texture::BindToTextureSlot(uint32 slot)
{
  GLCall(glActiveTexture(GL_TEXTURE0 + slot));
  GLCall(glBindTexture(GL_TEXTURE_2D, _id));
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

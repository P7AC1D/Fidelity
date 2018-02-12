#include "Texture.h"

#include "OpenGL.h"

namespace Rendering {
static uint32 ActiveTexture = -1;

GLenum CastWrapMethod(WrapMethod wrapMethod)
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

GLenum CastMagFilter(MagFilter magFilter)
{
  switch (magFilter)
  {
    case MagFilter::Nearest: return GL_NEAREST;
    case MagFilter::Linear: return GL_LINEAR;
    default: return GL_NEAREST;
  }
}

GLenum CastMinFilter(MinFilter minFilter)
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

Texture::Texture(TextureDesc desc) :
  Texture(desc, nullptr)
{  
}

Texture::Texture(TextureDesc desc, ubyte* data) :
  _desc(desc)
{
  Intialize(data);
}

Texture::~Texture()
{
  Unbind();
  GLCall(glDeleteTextures(1, &_id));
}

void Texture::SetWrapMethod(WrapMethod wrapMethod)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, CastWrapMethod(wrapMethod)));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, CastWrapMethod(wrapMethod)));
}

void Texture::SetMinFilter(MinFilter minFilter)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, CastMinFilter(minFilter)));
}

void Texture::SetMagFilter(MagFilter magFilter)
{
  Bind();
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, CastMagFilter(magFilter)));
}

void Texture::GenerateMipMaps()
{
  Bind();
  GLCall(glGenerateMipmap(GL_TEXTURE_2D));
}

void Texture::BindToTextureSlot(uint32 slot)
{
  GLCall(glActiveTexture(GL_TEXTURE0 + slot));
  Bind();
}

void Texture::Intialize(ubyte* data)
{
  GLCall(glGenTextures(1, &_id));
  Bind();

  switch (_desc.Format)
  {
    case PixelFormat::R8: 
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _desc.Width, _desc.Height, 0, GL_RED, GL_UNSIGNED_BYTE, data));
      break;
    case PixelFormat::RG8:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, _desc.Width, _desc.Height, 0, GL_RG, GL_UNSIGNED_BYTE, data));
      break;
    case PixelFormat::RGB8:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _desc.Width, _desc.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
      break;
    case PixelFormat::RGB16F:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _desc.Width, _desc.Height, 0, GL_RGB, GL_FLOAT, data));
      break;
    case PixelFormat::RGBA16F:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _desc.Width, _desc.Height, 0, GL_RGBA, GL_FLOAT, data));
      break;
    case PixelFormat::D32:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, _desc.Width, _desc.Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data));
      break;
    case PixelFormat::D24S8:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, _desc.Width, _desc.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data));
      break;
    case PixelFormat::RGBA8:
    default:
      GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _desc.Width, _desc.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
  }

  SetMinFilter(_desc.MinFilter);
  SetMagFilter(_desc.MagFilter);
  SetWrapMethod(_desc.WrapMethod);
}

void Texture::Bind()
{
  if (ActiveTexture != _id)
  {    
    GLCall(glBindTexture(GL_TEXTURE_2D, _id));
    ActiveTexture = _id;
  }
}

void Texture::Unbind()
{
  if (ActiveTexture == _id)
  {
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    ActiveTexture = -1;
  }
}
}

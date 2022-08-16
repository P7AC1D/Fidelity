#include "GLTexture.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

void GetInternalPixelFormat(TextureFormat textureFormat, GLenum &internalFormat, GLenum &format, GLenum &type, bool gammaCorrected)
{
  switch (textureFormat)
  {
  case TextureFormat::R8:
  {
    internalFormat = format = GL_RED;
    type = GL_UNSIGNED_BYTE;
    break;
  }
  case TextureFormat::RG8:
  {
    internalFormat = format = GL_RG;
    type = GL_UNSIGNED_BYTE;
    break;
  }
  case TextureFormat::RGB8:
  {
    format = GL_RGB;
    internalFormat = gammaCorrected ? GL_SRGB : GL_RGB;
    type = GL_UNSIGNED_BYTE;
    break;
  }
  case TextureFormat::RGBA8:
  {
    format = GL_RGBA;
    internalFormat = gammaCorrected ? GL_SRGB_ALPHA : GL_RGBA;
    type = GL_UNSIGNED_BYTE;
    break;
  }
  case TextureFormat::RGB16F:
  {
    internalFormat = GL_RGB16F;
    format = GL_RGB;
    type = GL_FLOAT;
    break;
  }
  case TextureFormat::RGBA16F:
  {
    internalFormat = GL_RGBA16F;
    format = GL_RGBA;
    type = GL_FLOAT;
    break;
  }
  case TextureFormat::D32:
  {
    internalFormat = GL_DEPTH_COMPONENT32;
    format = GL_DEPTH_COMPONENT;
    type = GL_UNSIGNED_BYTE;
    break;
  }
  case TextureFormat::D24:
  {
    internalFormat = GL_DEPTH_COMPONENT24;
    format = GL_DEPTH_COMPONENT;
    type = GL_UNSIGNED_BYTE;
    break;
  }
  case TextureFormat::D24S8:
  {
    internalFormat = GL_DEPTH24_STENCIL8;
    format = GL_DEPTH_STENCIL;
    type = GL_UNSIGNED_INT_24_8;
    break;
  }
  default:
    throw std::runtime_error("Unsupported TextureFormat");
  }
}

GLenum GetTextureTarget(TextureType textureType)
{
  switch (textureType)
  {
  case TextureType::Texture1D:
    return GL_TEXTURE_1D;
  case TextureType::Texture1DArray:
    return GL_TEXTURE_1D_ARRAY;
  case TextureType::Texture2D:
    return GL_TEXTURE_2D;
  case TextureType::Texture2DArray:
    return GL_TEXTURE_2D_ARRAY;
  case TextureType::Texture3D:
    return GL_TEXTURE_3D;
  case TextureType::TextureCube:
    return GL_TEXTURE_CUBE_MAP;
  default:
    throw std::runtime_error("Unsupported TextureType");
  }
}

GLenum GetTextureBindingTarget(TextureType textureType)
{
  switch (textureType)
  {
  case TextureType::Texture1D:
    return GL_TEXTURE_BINDING_1D;
  case TextureType::Texture1DArray:
    return GL_TEXTURE_BINDING_1D_ARRAY;
  case TextureType::Texture2D:
    return GL_TEXTURE_BINDING_2D;
  case TextureType::Texture2DArray:
    return GL_TEXTURE_BINDING_2D_ARRAY;
  case TextureType::Texture3D:
    return GL_TEXTURE_BINDING_3D;
  case TextureType::TextureCube:
    return GL_TEXTURE_BINDING_CUBE_MAP;
  default:
    throw std::runtime_error("Unsupported TextureType");
  }
}

GLTexture::~GLTexture()
{
  if (_id != 0)
  {
    GLCall(glDeleteTextures(1, &_id));
  }
}

void GLTexture::WriteData(uint32 mipLevel, uint32 face, const std::shared_ptr<ImageData> &data)
{
  ASSERT_TRUE(_desc.Width == data->GetWidth(), "Image width must be consistent with Texture");
  ASSERT_TRUE(_desc.Height == data->GetHeight(), "Image height must be consistent with Texture");
  ASSERT_TRUE(_desc.Depth == data->GetDepth(), "Image depth must be consistent with Texture");

  GLenum internalFormat;
  GLenum format;
  GLenum type;
  GetInternalPixelFormat(_desc.Format, internalFormat, format, type, _gammaCorrected);
  GLenum target = GetTextureTarget(_desc.Type);
  const auto &pixelData = data->GetPixelData();

  GLint previouslyBoundTexture = 0;
  GLCall(glGetIntegerv(GetTextureBindingTarget(_desc.Type), &previouslyBoundTexture));

  GLCall(glBindTexture(target, _id));
  switch (_desc.Type)
  {
  case TextureType::Texture1D:
    GLCall(glTexSubImage1D(target, mipLevel, data->GetLeft(), data->GetWidth(), format, type, &pixelData[0]));
    break;
  case TextureType::Texture1DArray:
    GLCall(glTexSubImage2D(target, mipLevel, data->GetLeft(), 0, data->GetWidth(), face, format, type, &pixelData[0]));
    break;
  case TextureType::Texture2D:
    GLCall(glTexSubImage2D(target, mipLevel, data->GetLeft(), data->GetBottom(), data->GetWidth(), data->GetHeight(), format, type, &pixelData[0]));
    break;
  case TextureType::Texture2DArray:
    GLCall(glTexSubImage3D(target, mipLevel, data->GetLeft(), data->GetBottom(), 0, data->GetWidth(), data->GetHeight(), face, format, type, &pixelData[0]));
    break;
  case TextureType::Texture3D:
    GLCall(glTexSubImage3D(target, mipLevel, data->GetLeft(), data->GetBottom(), data->GetBack(), data->GetWidth(), data->GetHeight(), data->GetDepth(), format, type, &pixelData[0]));
    break;
  case TextureType::TextureCube:
    GLCall(glTexSubImage2D(target, mipLevel, data->GetLeft(), 0, data->GetWidth(), face, format, type, &pixelData[0]));
    break;
  default:
    throw std::runtime_error("Unsupported TextureType");
  }
  GLCall(glBindTexture(target, previouslyBoundTexture));
}

void GLTexture::GenerateMips()
{
  GLint previouslyBoundTexture = 0;
  GLCall(glGetIntegerv(GetTextureBindingTarget(_desc.Type), &previouslyBoundTexture));

  GLenum target = GetTextureTarget(_desc.Type);
  GLCall(glBindTexture(target, _id));
  GLCall(glGenerateMipmap(target));

  GLCall(glBindTexture(target, previouslyBoundTexture));
}

GLTexture::GLTexture(const TextureDesc &desc, bool gammaCorrected) : Texture(desc, gammaCorrected), _id(0)
{
  Initialize();
}

void GLTexture::Initialize()
{
  if (IsInitialized())
  {
    return;
  }

  GLint previouslyBoundTexture = 0;
  GLCall(glGetIntegerv(GetTextureBindingTarget(_desc.Type), &previouslyBoundTexture));

  GLCall(glGenTextures(1, &_id));
  ASSERT_FALSE(_id == 0, "Could not generate texture object");

  auto target = GetTextureTarget(GetTextureType());
  GLCall(glBindTexture(target, _id));
  Allocate();

  _isInitialized = true;
  GLCall(glBindTexture(target, previouslyBoundTexture));
}

void GLTexture::Allocate()
{
  GLenum internalFormat;
  GLenum format;
  GLenum type;
  GetInternalPixelFormat(_desc.Format, internalFormat, format, type, _gammaCorrected);
  switch (_desc.Type)
  {
  case TextureType::Texture1D:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      GLCall(glTexImage1D(GL_TEXTURE_1D, i, internalFormat, _desc.Width, 0, format, type, nullptr));
    }
    break;
  case TextureType::Texture1DArray:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      GLCall(glTexImage2D(GL_TEXTURE_1D_ARRAY, i, internalFormat, _desc.Width, _desc.Count, 0, format, type, nullptr));
    }
    break;
  case TextureType::Texture2D:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      GLCall(glTexImage2D(GL_TEXTURE_2D, i, internalFormat, _desc.Width, _desc.Height, 0, format, type, nullptr));
    }
    break;
  case TextureType::Texture2DArray:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      GLCall(glTexImage3D(GL_TEXTURE_2D_ARRAY, i, internalFormat, _desc.Width, _desc.Height, _desc.Count, 0, format, type, nullptr));
    }
    break;
  case TextureType::TextureCube:
    for (uint32 face = 0; face < 6; face++)
    {
      for (uint32 i = 0; i < _desc.MipLevels; i++)
      {
        GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, i, internalFormat, _desc.Width, _desc.Height, 0, format, type, nullptr));
      }
    }
    break;
  case TextureType::Texture3D:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      GLCall(glTexImage3D(GL_TEXTURE_3D, i, internalFormat, _desc.Width, _desc.Height, _desc.Depth, 0, format, type, nullptr));
    }
  default:
    throw std::runtime_error("Unsupported TextureType");
  }
}

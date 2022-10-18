#include "GLTexture.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

void getInternalPixelFormat(TextureFormat textureFormat, GLenum &internalFormat, GLenum &format, GLenum &type, bool gammaCorrected)
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
  case TextureFormat::RGB32F:
  {
    internalFormat = GL_RGB32F;
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
  case TextureFormat::D32F:
  {
    internalFormat = GL_DEPTH_COMPONENT32F;
    format = GL_DEPTH_COMPONENT;
    type = GL_FLOAT;
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

GLenum getTextureTarget(TextureType textureType)
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

GLenum getTextureBindingTarget(TextureType textureType)
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
    glCall(glDeleteTextures(1, &_id));
  }
}

void GLTexture::writeData(uint32 mipLevel, uint32 face, const std::shared_ptr<ImageData> &data)
{
  ASSERT_TRUE(_desc.Width == data->getWidth(), "Image width must be consistent with Texture");
  ASSERT_TRUE(_desc.Height == data->getHeight(), "Image height must be consistent with Texture");
  ASSERT_TRUE(_desc.Depth == data->getDepth(), "Image depth must be consistent with Texture");

  GLenum internalFormat;
  GLenum format;
  GLenum type;
  getInternalPixelFormat(_desc.Format, internalFormat, format, type, _gammaCorrected);
  GLenum target = getTextureTarget(_desc.Type);
  const auto &pixelData = data->getPixelData();

  GLint previouslyBoundTexture = 0;
  glCall(glGetIntegerv(getTextureBindingTarget(_desc.Type), &previouslyBoundTexture));

  glCall(glBindTexture(target, _id));
  switch (_desc.Type)
  {
  case TextureType::Texture1D:
    glCall(glTexSubImage1D(target, mipLevel, data->getLeft(), data->getWidth(), format, type, &pixelData[0]));
    break;
  case TextureType::Texture1DArray:
    glCall(glTexSubImage2D(target, mipLevel, data->getLeft(), 0, data->getWidth(), face, format, type, &pixelData[0]));
    break;
  case TextureType::Texture2D:
    glCall(glTexSubImage2D(target, mipLevel, data->getLeft(), data->getBottom(), data->getWidth(), data->getHeight(), format, type, &pixelData[0]));
    break;
  case TextureType::Texture2DArray:
    glCall(glTexSubImage3D(target, mipLevel, data->getLeft(), data->getBottom(), 0, data->getWidth(), data->getHeight(), face, format, type, &pixelData[0]));
    break;
  case TextureType::Texture3D:
    glCall(glTexSubImage3D(target, mipLevel, data->getLeft(), data->getBottom(), data->getBack(), data->getWidth(), data->getHeight(), data->getDepth(), format, type, &pixelData[0]));
    break;
  case TextureType::TextureCube:
    glCall(glTexSubImage2D(target, mipLevel, data->getLeft(), 0, data->getWidth(), face, format, type, &pixelData[0]));
    break;
  default:
    throw std::runtime_error("Unsupported TextureType");
  }
  glCall(glBindTexture(target, previouslyBoundTexture));
}

void GLTexture::writeData(uint32 mipLevel, uint32 face, uint32 xStart, uint32 xCount, uint32 yStart, uint32 yCount, uint32 zStart, uint32 zCount, void *data)
{
  ASSERT_TRUE(_desc.Width <= xCount - xStart, "Width pixel data count exceeds texture width.");
  ASSERT_TRUE(_desc.Height <= yCount - yStart, "Height pixel data count exceeds texture width.");

  GLenum internalFormat;
  GLenum format;
  GLenum type;
  getInternalPixelFormat(_desc.Format, internalFormat, format, type, _gammaCorrected);
  GLenum target = getTextureTarget(_desc.Type);

  GLint previouslyBoundTexture = 0;
  glCall(glGetIntegerv(getTextureBindingTarget(_desc.Type), &previouslyBoundTexture));

  glCall(glBindTexture(target, _id));
  switch (_desc.Type)
  {
  case TextureType::Texture1D:
    glCall(glTexSubImage1D(target, mipLevel, xStart, xCount, format, type, data));
    break;
  case TextureType::Texture1DArray:
    glCall(glTexSubImage2D(target, mipLevel, xStart, yStart, xCount, yCount, format, type, data));
    break;
  case TextureType::Texture2D:
    glCall(glTexSubImage2D(target, mipLevel, xStart, yStart, xCount, yCount, format, type, data));
    break;
  case TextureType::Texture2DArray:
    glCall(glTexSubImage3D(target, mipLevel, xStart, yStart, zStart, xCount, yCount, zCount, format, type, data));
    break;
  case TextureType::Texture3D:
    glCall(glTexSubImage3D(target, mipLevel, xStart, yStart, zStart, xCount, yCount, zCount, format, type, data));
    break;
  case TextureType::TextureCube:
    glCall(glTexSubImage2D(target, mipLevel, xStart, 0, yStart, face, format, type, data));
    break;
  default:
    throw std::runtime_error("Unsupported TextureType");
  }
  glCall(glBindTexture(target, previouslyBoundTexture));
}

void GLTexture::generateMips()
{
  GLint previouslyBoundTexture = 0;
  glCall(glGetIntegerv(getTextureBindingTarget(_desc.Type), &previouslyBoundTexture));

  GLenum target = getTextureTarget(_desc.Type);
  glCall(glBindTexture(target, _id));
  glCall(glGenerateMipmap(target));

  glCall(glBindTexture(target, previouslyBoundTexture));
}

GLTexture::GLTexture(const TextureDesc &desc, bool gammaCorrected) : Texture(desc, gammaCorrected), _id(0)
{
  Initialize();
}

void GLTexture::Initialize()
{
  if (isInitialized())
  {
    return;
  }

  GLint previouslyBoundTexture = 0;
  glCall(glGetIntegerv(getTextureBindingTarget(_desc.Type), &previouslyBoundTexture));

  glCall(glGenTextures(1, &_id));
  ASSERT_FALSE(_id == 0, "Could not generate texture object");

  auto target = getTextureTarget(getTextureType());
  glCall(glBindTexture(target, _id));
  Allocate();

  _isInitialized = true;
  glCall(glBindTexture(target, previouslyBoundTexture));
}

void GLTexture::Allocate()
{
  GLenum internalFormat;
  GLenum format;
  GLenum type;
  getInternalPixelFormat(_desc.Format, internalFormat, format, type, _gammaCorrected);
  switch (_desc.Type)
  {
  case TextureType::Texture1D:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      glCall(glTexImage1D(GL_TEXTURE_1D, i, internalFormat, _desc.Width, 0, format, type, nullptr));
    }
    break;
  case TextureType::Texture1DArray:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      glCall(glTexImage2D(GL_TEXTURE_1D_ARRAY, i, internalFormat, _desc.Width, _desc.Count, 0, format, type, nullptr));
    }
    break;
  case TextureType::Texture2D:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      glCall(glTexImage2D(GL_TEXTURE_2D, i, internalFormat, _desc.Width, _desc.Height, 0, format, type, nullptr));
    }
    break;
  case TextureType::Texture2DArray:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      glCall(glTexImage3D(GL_TEXTURE_2D_ARRAY, i, internalFormat, _desc.Width, _desc.Height, _desc.Count, 0, format, type, nullptr));
    }
    break;
  case TextureType::TextureCube:
    for (uint32 face = 0; face < 6; face++)
    {
      for (uint32 i = 0; i < _desc.MipLevels; i++)
      {
        glCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, i, internalFormat, _desc.Width, _desc.Height, 0, format, type, nullptr));
      }
    }
    break;
  case TextureType::Texture3D:
    for (uint32 i = 0; i < _desc.MipLevels; i++)
    {
      glCall(glTexImage3D(GL_TEXTURE_3D, i, internalFormat, _desc.Width, _desc.Height, _desc.Depth, 0, format, type, nullptr));
    }
  default:
    throw std::runtime_error("Unsupported TextureType");
  }
}

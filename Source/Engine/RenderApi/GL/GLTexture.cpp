#include "GLTexture.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

void GetInternalPixelFormat(TextureFormat textureFormat, GLenum& internalFormat, GLenum& format, GLenum& type)
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
      internalFormat = format = GL_RGB;
      type = GL_UNSIGNED_BYTE;
      break;
    }
    case TextureFormat::RGBA8:
    {
      internalFormat = format = GL_RGBA;
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
      internalFormat = GL_DEPTH_COMPONENT32F;
      format = GL_DEPTH_COMPONENT;
      type = GL_FLOAT;
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
    case TextureType::Texture1D: return GL_TEXTURE_1D;
    case TextureType::Texture1DArray: return GL_TEXTURE_1D_ARRAY;
    case TextureType::Texture2D: return GL_TEXTURE_2D;
    case TextureType::Texture2DArray: return GL_TEXTURE_2D_ARRAY;
    case TextureType::Texture3D: return GL_TEXTURE_3D;
    case TextureType::TextureCube: return GL_TEXTURE_CUBE_MAP;
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

void GLTexture::WriteData(uint32 mipLevel, uint32 face, const std::shared_ptr<ImageData>& data)
{
  Assert::ThrowIfFalse(_desc.Width == data->GetWidth(), "Image width must be consistent with Texture");
  Assert::ThrowIfFalse(_desc.Height == data->GetHeight(), "Image height must be consistent with Texture");
  Assert::ThrowIfFalse(_desc.Depth == data->GetDepth(), "Image depth must be consistent with Texture");
  
  GLenum internalFormat;
  GLenum format;
  GLenum type;
  GetInternalPixelFormat(_desc.Format, internalFormat, format, type);
  GLenum target = GetTextureTarget(_desc.Type);
  
  const auto& pixelData = data->GetPixelData();
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
}

void GLTexture::GenerateMips()
{
  GLenum target = GetTextureTarget(_desc.Type);
  GLCall(glBindTexture(target, _id));
  GLCall(glGenerateMipmap(target));
}

GLTexture::GLTexture(const TextureDesc& desc): Texture(desc), _id(0)
{
  Initialize();
}

void GLTexture::Initialize()
{
  if (IsInitialized())
  {
    return;
  }
  
  GLCall(glGenTextures(1, &_id));
  Assert::ThrowIfTrue(_id == 0, "Could not generate texture object");
  
  GLCall(glBindTexture(GetTextureTarget(_desc.Type), _id));
  Allocate();
  
  _isInitialized = true;
}

void GLTexture::Allocate()
{
  GLenum internalFormat;
  GLenum format;
  GLenum type;
  GetInternalPixelFormat(_desc.Format, internalFormat, format, type);
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

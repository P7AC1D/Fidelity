#include "GLSamplerState.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

GLenum GetTextureAddressingMode(TextureAddressMode addressingMode)
{
  switch (addressingMode)
  {
    case TextureAddressMode::Clamp: return GL_CLAMP_TO_EDGE;
    case TextureAddressMode::Mirror: return GL_MIRRORED_REPEAT;
    case TextureAddressMode::Wrap: return GL_REPEAT;
    case TextureAddressMode::Border: return GL_CLAMP_TO_BORDER;
    default: return GL_CLAMP_TO_BORDER;
  }
}

GLSamplerState::~GLSamplerState()
{
  if (_id != 0)
  {
    GLCall(glDeleteSamplers(1, &_id));
  }
}

GLSamplerState::GLSamplerState(const SamplerStateDesc& desc): SamplerState(desc), _id(0), _initalized(false)
{
  Initialize();
}

void GLSamplerState::Initialize()
{
  if (IsInitialized())
  {
    return;
  }
  
  GLCall(glGenSamplers(1, &_id));
  Assert::ThrowIfTrue(_id == 0, "Could not generate GL sampler object");
  
  SetTextureAddressingMode(GetAddressingMode());
  SetTextureMinMipFiltering(GetMinFilteringMode(), GetMipFilteringMode());
  SetTextureMagFiltering(GetMagFilteringMode());
}


void GLSamplerState::SetTextureAddressingMode(AddressingMode addressingMode)
{
  if (IsInitialized())
  {
    return;
  }
  
  GLCall(glSamplerParameteri(_id, GL_TEXTURE_WRAP_R, GetTextureAddressingMode(addressingMode.W)));
  GLCall(glSamplerParameteri(_id, GL_TEXTURE_WRAP_S, GetTextureAddressingMode(addressingMode.U)));
  GLCall(glSamplerParameteri(_id, GL_TEXTURE_WRAP_T, GetTextureAddressingMode(addressingMode.V)));
}

void GLSamplerState::SetTextureMinMipFiltering(TextureFilteringMode minFilteringMode, TextureFilteringMode mipFilteringMode)
{
  if (IsInitialized())
  {
    return;
  }
  
  switch (minFilteringMode)
  {
    case TextureFilteringMode::None:
    case TextureFilteringMode::Point:
      switch (mipFilteringMode)
    {
      case TextureFilteringMode::None:
        GLCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        break;
      case TextureFilteringMode::Point:
        GLCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
        break;
      case TextureFilteringMode::Linear:
      case TextureFilteringMode::Anisotropic:
        GLCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        break;
      default:
        break;
    }
      break;
      
    case TextureFilteringMode::Linear:
    case TextureFilteringMode::Anisotropic:
      switch (mipFilteringMode)
    {
      case TextureFilteringMode::None:
        GLCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        break;
      case TextureFilteringMode::Point:
        GLCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
        break;
      case TextureFilteringMode::Linear:
      case TextureFilteringMode::Anisotropic:
        GLCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
        break;
      default:
        break;
    }
      
    default:
      break;
  }
}

void GLSamplerState::SetTextureMagFiltering(TextureFilteringMode textureFilteringMode)
{
  if (IsInitialized())
  {
    return;
  }
  
  switch (textureFilteringMode)
  {
    case TextureFilteringMode::None:
    case TextureFilteringMode::Point:
      GLCall(glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
      break;
    case TextureFilteringMode::Linear:
    case TextureFilteringMode::Anisotropic:
      GLCall(glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    default:
      break;
  }
}

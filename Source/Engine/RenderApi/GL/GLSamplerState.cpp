#include "GLSamplerState.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

GLenum getTextureAddressingMode(TextureAddressMode addressingMode)
{
  switch (addressingMode)
  {
  case TextureAddressMode::Clamp:
    return GL_CLAMP_TO_EDGE;
  case TextureAddressMode::Mirror:
    return GL_MIRRORED_REPEAT;
  case TextureAddressMode::Wrap:
    return GL_REPEAT;
  case TextureAddressMode::Border:
    return GL_CLAMP_TO_BORDER;
  default:
    return GL_CLAMP_TO_BORDER;
  }
}

GLSamplerState::~GLSamplerState()
{
  if (_id != 0)
  {
    glCall(glDeleteSamplers(1, &_id));
  }
}

GLSamplerState::GLSamplerState(const SamplerStateDesc &desc) : SamplerState(desc), _id(0), _initalized(false)
{
  initialize();
}

void GLSamplerState::initialize()
{
  if (isInitialized())
  {
    return;
  }

  glCall(glGenSamplers(1, &_id));
  ASSERT_FALSE(_id == 0, "Could not generate GL sampler object");

  setTextureAddressingMode(getAddressingMode());
  setTextureMinMipFiltering(getMinFilteringMode(), getMipFilteringMode());
  setTextureMagFiltering(getMagFilteringMode());
  setBorderColour(getBorderColour());
}

void GLSamplerState::setTextureAddressingMode(AddressingMode addressingMode)
{
  if (isInitialized())
  {
    return;
  }

  glCall(glSamplerParameteri(_id, GL_TEXTURE_WRAP_R, getTextureAddressingMode(addressingMode.W)));
  glCall(glSamplerParameteri(_id, GL_TEXTURE_WRAP_S, getTextureAddressingMode(addressingMode.U)));
  glCall(glSamplerParameteri(_id, GL_TEXTURE_WRAP_T, getTextureAddressingMode(addressingMode.V)));
}

void GLSamplerState::setTextureMinMipFiltering(TextureFilteringMode minFilteringMode, TextureFilteringMode mipFilteringMode)
{
  if (isInitialized())
  {
    return;
  }

  switch (minFilteringMode)
  {
  case TextureFilteringMode::None:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    break;
  case TextureFilteringMode::Point:
    switch (mipFilteringMode)
    {
    case TextureFilteringMode::None:
      glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      break;
    case TextureFilteringMode::Point:
      glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
      break;
    case TextureFilteringMode::Linear:
    case TextureFilteringMode::Anisotropic:
      glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
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
      glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
      break;
    case TextureFilteringMode::Point:
      glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
      break;
    case TextureFilteringMode::Linear:
    case TextureFilteringMode::Anisotropic:
      glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
      break;
    default:
      break;
    }

  default:
    break;
  }
}

void GLSamplerState::setTextureMagFiltering(TextureFilteringMode textureFilteringMode)
{
  if (isInitialized())
  {
    return;
  }

  switch (textureFilteringMode)
  {
  case TextureFilteringMode::None:
  case TextureFilteringMode::Point:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    break;
  case TextureFilteringMode::Linear:
  case TextureFilteringMode::Anisotropic:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  default:
    break;
  }
}

void GLSamplerState::setBorderColour(Colour borderColour)
{
  const float32 colour[4] = {borderColour[0], borderColour[1], borderColour[2], borderColour[3]};
  glCall(glSamplerParameterfv(_id, GL_TEXTURE_BORDER_COLOR, colour));
}
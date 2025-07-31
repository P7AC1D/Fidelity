#include "GLSamplerState.hpp"

#include <algorithm>
#include <cstring>
#include "../../Utility/Assert.hpp"
#include "GL.hpp"

// GL_EXT_texture_filter_anisotropic extension constants
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

GLenum getTextureAddressingMode(TextureAddressMode addressingMode)
{
  switch (addressingMode)
  {
  case TextureAddressMode::ClampToEdge:
    return GL_CLAMP_TO_EDGE;
  case TextureAddressMode::MirroredRepeat:
    return GL_MIRRORED_REPEAT;
  case TextureAddressMode::Repeat:
    return GL_REPEAT;
  case TextureAddressMode::ClampToBorder:
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

void *GLSamplerState::getNativeHandle() const
{
  if (!isValid())
  {
    throw std::runtime_error("Sampler state is not valid");
  }

  return reinterpret_cast<void *>(static_cast<uintptr_t>(_id));
}

bool GLSamplerState::isValid() const
{
  return _id != 0 && _initalized;
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
  setTextureMinMipFiltering(getMinFilteringMode());
  setTextureMagFiltering(getMagFilteringMode());
  setBorderColour(getBorderColour());
  setAnisotropicFiltering(getMaxAnisotropy());

  _initalized = true;
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

void GLSamplerState::setTextureMinMipFiltering(TextureFilteringMode minFilteringMode)
{
  if (isInitialized())
  {
    return;
  }

  switch (minFilteringMode)
  {
  case TextureFilteringMode::Nearest:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    break;
  case TextureFilteringMode::Linear:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    break;
  case TextureFilteringMode::NearestMipNearest:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
    break;
  case TextureFilteringMode::LinearMipNearest:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
    break;
  case TextureFilteringMode::NearestMipLinear:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    break;
  case TextureFilteringMode::LinearMipLinear:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    break;
  default:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
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
  case TextureFilteringMode::Nearest:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    break;
  case TextureFilteringMode::Linear:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    break;
  default:
    glCall(glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    break;
  }
}

void GLSamplerState::setBorderColour(Colour borderColour)
{
  const float32 colour[4] = {borderColour[0], borderColour[1], borderColour[2], borderColour[3]};
  glCall(glSamplerParameterfv(_id, GL_TEXTURE_BORDER_COLOR, colour));
}

void GLSamplerState::setAnisotropicFiltering(float maxAnisotropy)
{
  if (isInitialized())
  {
    return;
  }

  // Check if anisotropic filtering extension is available
  if (maxAnisotropy > 1.0f)
  {
    // Check for EXT_texture_filter_anisotropic extension
    static bool extensionChecked = false;
    static bool hasAnisotropicExtension = false;

    if (!extensionChecked)
    {
      const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
      hasAnisotropicExtension = (extensions && strstr(extensions, "GL_EXT_texture_filter_anisotropic") != nullptr);
      extensionChecked = true;
    }

    if (hasAnisotropicExtension)
    {
      // Get maximum supported anisotropy level
      float maxSupportedAnisotropy = 1.0f;
      glCall(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSupportedAnisotropy));

      // Clamp to supported range
      float clampedAnisotropy = std::min(maxAnisotropy, maxSupportedAnisotropy);
      glCall(glSamplerParameterf(_id, GL_TEXTURE_MAX_ANISOTROPY_EXT, clampedAnisotropy));
    }
  }
}
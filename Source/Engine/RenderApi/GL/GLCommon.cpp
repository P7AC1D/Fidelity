#include "GLCommon.hpp"
using namespace MemoryDeps;

GLenum getTextureTargetFromType(TextureType textureType)
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
  case TextureType::TextureCubeArray:
    return GL_TEXTURE_CUBE_MAP_ARRAY;
  default:
    return GL_TEXTURE_2D;
  }
}

GLenum getPrimitiveTopology(PrimitiveTopology topology)
{
  switch (topology)
  {
  case PrimitiveTopology::PointList:
    return GL_POINTS;
  case PrimitiveTopology::LineList:
    return GL_LINES;
  case PrimitiveTopology::TriangleList:
    return GL_TRIANGLES;
  case PrimitiveTopology::TriangleStrip:
    return GL_TRIANGLE_STRIP;
  case PrimitiveTopology::PatchList:
    return GL_PATCHES;
  default:
    return GL_TRIANGLES;
  }
}

GLenum getStencilOp(StencilOperation stencilOperation, bool invert)
{
  switch (stencilOperation)
  {
  case StencilOperation::Keep:
    return GL_KEEP;
  case StencilOperation::Zero:
    return GL_ZERO;
  case StencilOperation::Replace:
    return GL_REPLACE;
  case StencilOperation::Incr:
    return invert ? GL_DECR : GL_INCR;
  case StencilOperation::Decr:
    return invert ? GL_INCR : GL_DECR;
  case StencilOperation::IncrSat:
    return invert ? GL_DECR_WRAP : GL_INCR_WRAP;
  case StencilOperation::DescSat:
    return invert ? GL_INCR_WRAP : GL_DECR_WRAP;
  case StencilOperation::Invert:
    return GL_INVERT;
  default:
    return GL_KEEP;
  }
}

GLenum getCompareFunc(ComparisonFunction func)
{
  switch (func)
  {
  case ComparisonFunction::Never:
    return GL_NEVER;
  case ComparisonFunction::Less:
    return GL_LESS;
  case ComparisonFunction::Equal:
    return GL_EQUAL;
  case ComparisonFunction::LessEqual:
    return GL_LEQUAL;
  case ComparisonFunction::Greater:
    return GL_GREATER;
  case ComparisonFunction::NotEqual:
    return GL_NOTEQUAL;
  case ComparisonFunction::GreaterEqual:
    return GL_GEQUAL;
  case ComparisonFunction::Always:
    return GL_ALWAYS;
  default:
    return GL_ALWAYS;
  }
}

GLenum getBlendFactor(BlendFactor factor)
{
  switch (factor)
  {
  case BlendFactor::Zero:
    return GL_ZERO;
  case BlendFactor::One:
    return GL_ONE;
  case BlendFactor::SrcColour:
    return GL_SRC_COLOR;
  case BlendFactor::InvSrcColour:
    return GL_ONE_MINUS_SRC_COLOR;
  case BlendFactor::SrcAlpha:
    return GL_SRC_ALPHA;
  case BlendFactor::InvSrcAlpha:
    return GL_ONE_MINUS_SRC_ALPHA;
  case BlendFactor::DestAlpha:
    return GL_DST_ALPHA;
  case BlendFactor::InvDestAlpha:
    return GL_ONE_MINUS_DST_ALPHA;
  case BlendFactor::DestColour:
    return GL_DST_COLOR;
  case BlendFactor::InvDestColour:
    return GL_ONE_MINUS_DST_COLOR;
  default:
    return GL_ONE;
  }
}

GLenum getBlendOp(BlendOperation op)
{
  switch (op)
  {
  case BlendOperation::Add:
    return GL_FUNC_ADD;
  case BlendOperation::Subtract:
    return GL_FUNC_SUBTRACT;
  case BlendOperation::RevSubtract:
    return GL_FUNC_REVERSE_SUBTRACT;
  case BlendOperation::Min:
    return GL_MIN;
  case BlendOperation::Max:
    return GL_MAX;
  default:
    return GL_FUNC_ADD;
  }
}

bool glMemoryBarrierAvailable()
{
#if defined(GL_VERSION_4_2)
  // Only available when compiled with GL 4.2 headers and runtime exposes 4.2
  return GLAD_GL_VERSION_4_2 != 0;
#else
  // With a GL 4.1-only loader, glMemoryBarrier is not declared; treat as unavailable
  return false;
#endif
}

GLbitfield mapMemoryDepsToGL(uint32 memoryDeps)
{
  GLbitfield bits = 0;
#if defined(GL_VERSION_4_2) && defined(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT)
  if (memoryDeps & (ShaderWrite | StorageWrite | StorageRead))
    bits |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_UNIFORM_BARRIER_BIT)
  if (memoryDeps & (UniformRead))
    bits |= GL_UNIFORM_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_SHADER_STORAGE_BARRIER_BIT)
  if (memoryDeps & (ShaderRead | ShaderWrite | StorageRead | StorageWrite))
    bits |= GL_SHADER_STORAGE_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_TEXTURE_FETCH_BARRIER_BIT)
  if (memoryDeps & (ShaderRead))
    bits |= GL_TEXTURE_FETCH_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_FRAMEBUFFER_BARRIER_BIT)
  if (memoryDeps & (ColorAttachmentRead | ColorAttachmentWrite | DepthStencilRead | DepthStencilWrite))
    bits |= GL_FRAMEBUFFER_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_ELEMENT_ARRAY_BARRIER_BIT)
  if (memoryDeps & (TransferRead | TransferWrite))
    bits |= GL_ELEMENT_ARRAY_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_PIXEL_BUFFER_BARRIER_BIT)
  if (memoryDeps & (TransferRead | TransferWrite))
    bits |= GL_PIXEL_BUFFER_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_BUFFER_UPDATE_BARRIER_BIT)
  if (memoryDeps & (TransferWrite))
    bits |= GL_BUFFER_UPDATE_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT)
  if (memoryDeps & Host)
    bits |= GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT;
#endif
#if defined(GL_VERSION_4_2) && defined(GL_ALL_BARRIER_BITS)
  // If nothing mapped but deps requested, return ALL to be safe
  if (bits == 0 && memoryDeps != 0)
    bits = GL_ALL_BARRIER_BITS;
#endif
  return bits;
}
#include "GLCommon.hpp"

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
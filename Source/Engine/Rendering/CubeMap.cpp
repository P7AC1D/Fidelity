#include "CubeMap.h"

#include "OpenGL.h"

namespace Rendering
{
CubeMap::CubeMap()
{
  GLCall(glGenTextures(1, &_id));
}

CubeMap::~CubeMap()
{
  GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
  GLCall(glDeleteTextures(1, &_id));
}
}

#include "CubeMap.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif
#ifdef _WIN32
#include <GL/glew.h>
#endif

namespace Rendering
{
CubeMap::CubeMap()
{
  glGenTextures(1, &_id);
}

CubeMap::~CubeMap()
{
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  glDeleteTextures(1, &_id);
}
}

#include "OpenGL.h"

void GLClearError()
{
  while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const byte* function, const byte* file, int line)
{
  while (GLenum error = glGetError())
  {
    std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << std::endl;
    return false;
  }
  return true;
}
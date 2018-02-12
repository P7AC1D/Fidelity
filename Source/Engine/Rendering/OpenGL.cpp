#include "OpenGL.h"

#include <string>

#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502

std::string GLErrorToStr(GLenum error)
{
  switch (error)
  {
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default: std::to_string(static_cast<uint32>(error));
  }
}

void GLClearError()
{
  while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const byte* function, const byte* file, int line)
{
  while (GLenum error = glGetError())
  {
    auto glError = GLErrorToStr(error);
    std::cout << "[OpenGL Error] (" << glError << "): " << function << " " << file << ":" << std::endl;
    return false;
  }
  return true;
}
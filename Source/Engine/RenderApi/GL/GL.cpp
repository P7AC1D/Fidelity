#include "GL.hpp"

#include <string>
#include "../../Core/Application.h"

#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502

std::string glErrorToStr(GLenum error)
{
  switch (error)
  {
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  }
  return std::to_string(static_cast<uint32>(error));
}

void glClearError()
{
  while (glGetError() != GL_NO_ERROR)
    ;
}

bool glLogCall(const byte *function, const byte *file, int line)
{
  // std::cout << "Calling OpenGL function: " << function << " " << file << ":" << line << std::endl;
  while (GLenum error = glGetError())
  {
    auto glError = glErrorToStr(error);
    std::cout << "[OpenGL Error] (" << glError << "): " << function << " " << file << ":" << std::endl;
    return false;
  }
  return true;
}

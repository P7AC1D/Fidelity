#pragma once
#include <iostream>

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif
#ifdef _WIN32
  #include <GL/glew.h>
#endif

#include "../../Core/Types.hpp"

#ifdef __clang__
  #define ASSERT(x) if (!(x)) __asm__("int $3")
  #define GLCall(x) GLClearError();\
        x;\
        ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#endif
#ifdef _MSC_VER
  #define ASSERT(x) if (!(x)) __debugbreak();
  #ifdef _DEBUG
    #define GLCall(x) GLClearError();\
      x;\
      ASSERT(GLLogCall(#x, __FILE__, __LINE__))
  #else
    #define GLCall(x) (x)
  #endif
#endif

#define GLCall_ReturnVoidPtr(x, out) void* ptr = nullptr; \
ptr = (x); \
ASSERT(GLLogCall(#x, __FILE__, __LINE__)) \
ptr;\
(out) = ptr;\

#define GLCall_ReturnGLenum(x, out) GLenum data = 0; \
data = (x); \
ASSERT(GLLogCall(#x, __FILE__, __LINE__)) \
data;\
(out) = data;\

#define GLCall_ReturnGLuint(x, out) GLuint data = 0; \
data = (x); \
ASSERT(GLLogCall(#x, __FILE__, __LINE__)) \
data;\
(out) = data;\

#define GLCall_ReturnGLboolean(x, out) GLboolean data = GL_FALSE; \
data = (x); \
ASSERT(GLLogCall(#x, __FILE__, __LINE__)) \
data;\
(out) = data;\

void GLClearError();
bool GLLogCall(const byte* function, const byte* file, int line);

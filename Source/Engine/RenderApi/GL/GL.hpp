#pragma once
#include <iostream>

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif
#ifdef _WIN32
  #include <glad/glad.h>
#endif

#include "../../Core/Types.hpp"

#ifdef __clang__
#define ASSERT(x) if (!(x)) __asm__("int $3");
#endif
#ifdef _MSC_VER
#define ASSERT(x) if (!(x)) __debugbreak();
#endif

#ifdef NDEBUG
#define GLCall(x) GLClearError();\
x;\
ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
#define GLCall(x) x;
#endif

#ifdef NDEBUG
#define GLCall2(x, out) { decltype(out) data = out; \
data = (x); \
GLClearError();\
ASSERT(GLLogCall(#x, __FILE__, __LINE__)) \
data;\
(out) = data; }
#else
#define GLCall2(x, out) { decltype(out) data = out; \
data = (x); \
data;\
(out) = data; }
#endif

void GLClearError();
bool GLLogCall(const byte* function, const byte* file, int line);
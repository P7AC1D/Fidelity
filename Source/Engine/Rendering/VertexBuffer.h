#pragma once
#include <vector>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif
#ifdef _WIN32
#include <GL/glew.h>
#endif

#include "../Core/Types.hpp"

namespace Rendering
{
enum class BufferUsage : GLenum
{
  Static = GL_STATIC_DRAW,
  Dynamic = GL_DYNAMIC_DRAW
};

enum class BufferType : GLenum
{
  Array = GL_ARRAY_BUFFER,
  Texture = GL_TEXTURE_BUFFER,
  Uniform = GL_UNIFORM_BUFFER,
};

class VertexBuffer
{
public: 
  VertexBuffer(BufferType bufferType = BufferType::Array);
  ~VertexBuffer();

  template<typename T>
  void UploadData(const std::vector<T>& data, BufferUsage bufferUsage);

private:
  void UploadData(const void* dataPtr, int32 dataBytes, uint32 bufferUsage);

  VertexBuffer(VertexBuffer&) = delete;
  VertexBuffer(VertexBuffer&&) = delete;
  VertexBuffer& operator=(VertexBuffer&) = delete;
  VertexBuffer& operator=(VertexBuffer&&) = delete;

  uint32 _vaoId;
  BufferType _bufferType;

  friend class Renderer;
};

template<typename T>
inline void VertexBuffer::UploadData(const std::vector<T>& data, BufferUsage bufferUsage)
{
  UploadData(reinterpret_cast<const void*>(data.data()), static_cast<int32>(sizeof(T) * data.size()), static_cast<uint32>(bufferUsage));
}
}

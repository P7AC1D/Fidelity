#pragma once
#include <vector>

#include "../Core/Types.hpp"
#include "OpenGL.h"

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
  void UploadData(std::vector<T>& data, BufferUsage bufferUsage);

private:
  void UploadData(void* dataPtr, int32 dataBytes, uint32 bufferUsage);

  uint32 _vaoId;
  BufferType _bufferType;

  friend class Renderer;
};

template<typename T>
inline void VertexBuffer::UploadData(std::vector<T>& data, BufferUsage bufferUsage)
{
  UploadData(reinterpret_cast<void*>(&data[0]), static_cast<int32>(sizeof(T) * data.size()), static_cast<uint32>(bufferUsage));
}
}

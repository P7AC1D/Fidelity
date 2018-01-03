#pragma once
#include <vector>

#include <OpenGL/gl3.h>

#include "Types.h"

enum class VertexBufferUsage
{
  Static,
  Dynamic
};

class VertexBuffer
{
public:
  static 
  
  VertexBuffer();
  ~VertexBuffer();
  
  template <typename T>
  void UploadData(const std::vector<T>& data, VertexBufferUsage usage);
  void Bind() const;
  void Unbind() const;
  
  uint32 GetId() const { return _vbo; }
  
private:
  void UploadData(uint32 sizeInBytes, VertexBufferUsage usage, const void* data);
  GLenum GetBufferUsage(VertexBufferUsage usage);
  
private:
  uint32 _vbo;
  uint32 _vao;
};

template <typename T>
void VertexBuffer::UploadData(const std::vector<T>& data, VertexBufferUsage usage)
{
  UploadData(sizeof(T) * data.size(), usage, data.data());
}

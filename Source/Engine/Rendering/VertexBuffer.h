#pragma once
#include <vector>

#include "../Core/Types.hpp"

namespace Rendering
{
enum class BufferUsage
{
  Static,
  Dynamic,
};

class VertexBuffer
{
public: 
  VertexBuffer();
  ~VertexBuffer();

  inline uint32 GetId() const { return _vaoId; }

  void Bind() const;
  void Unbind() const;

  template<typename T>
  void UploadData(std::vector<T>& data, BufferUsage bufferUsage);

private:
  void UploadData(void* dataPtr, int32 dataBytes, BufferUsage bufferUsage);

  VertexBuffer(const VertexBuffer& buffer) = delete;
  VertexBuffer(const VertexBuffer&& buffer) = delete;
  VertexBuffer& operator=(const VertexBuffer& buffer) = delete;
  VertexBuffer& operator=(const VertexBuffer&& buffer) = delete;

private:
  uint32 _vaoId;
};

template<typename T>
inline void VertexBuffer::UploadData(std::vector<T>& data, BufferUsage bufferUsage)
{
  UploadData(reinterpret_cast<void*>(&data[0]), static_cast<int32>(sizeof(T) * data.size()), bufferUsage);
}
}

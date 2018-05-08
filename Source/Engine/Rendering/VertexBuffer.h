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

enum class VertexAttribType
{
  Vec2,
  Vec3,
  Vec4
};

struct VertexAttrib;

class VertexBuffer
{
public: 
  VertexBuffer();
  ~VertexBuffer();
  
  inline uint32 GetId() const { return _vao; }

  void Apply();
  void PushVertexAttrib(VertexAttribType vertexDeclType);
  void ResetVertexAttribs();

  template<typename T>
  void UploadData(const std::vector<T>& data, BufferUsage bufferUsage);

private:
  void Bind();
  void Unbind();
  void UploadData(const void* dataPtr, int32 dataBytes, BufferUsage bufferUsage);

  void RefreshVertexAttribs();

  VertexBuffer(const VertexBuffer& buffer) = delete;
  VertexBuffer(const VertexBuffer&& buffer) = delete;
  VertexBuffer& operator=(const VertexBuffer& buffer) = delete;
  VertexBuffer& operator=(const VertexBuffer&& buffer) = delete;

private:
  uint32 _vao;
  uint32 _vbo;
  uint32 _vertexAttributeCount;
  uint32 _vertexStrideBytes;
  uint32 _vertexOffsetBytes;
  bool _refreshVertexAttribs;
  std::vector<VertexAttrib> _vertexAttribs;
};

template<typename T>
inline void VertexBuffer::UploadData(const std::vector<T>& data, BufferUsage bufferUsage)
{
  UploadData(reinterpret_cast<const void*>(&data[0]), static_cast<int32>(sizeof(T) * data.size()), bufferUsage);
}
}

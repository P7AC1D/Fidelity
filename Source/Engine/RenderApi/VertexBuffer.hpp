#pragma once
#include "GpuBuffer.hpp"

struct VertexBufferDesc
{
  uint32 VertexSizeBytes;
  uint32 VertexCount;
  BufferUsage BufferUsage = BufferUsage::Default;
};

class VertexBuffer : public GpuBuffer
{
public:
  uint32 GetVertexSizeBytes() const { return _vertexSizeBytes; }
  uint32 GetVertexCount() const { return _vertexCount; }
  
protected:
  VertexBuffer(const VertexBufferDesc& desc):
    GpuBuffer({
      desc.VertexSizeBytes * desc.VertexCount,
      BufferType::Vertex,
      desc.BufferUsage
    }),
    _vertexSizeBytes(desc.VertexSizeBytes),
    _vertexCount(desc.VertexCount)
  {
  }
  
private:
  uint32 _vertexSizeBytes;
  uint32 _vertexCount;
};

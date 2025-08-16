#pragma once
#include "GpuBuffer.hpp"

/**
 * @brief Description of a vertex buffer resource.
 */
struct VertexBufferDesc
{
  uint64 VertexSizeBytes;
  uint32 VertexCount;
  BufferUsage BufferUsage = BufferUsage::Default;
};

/**
 * @brief GPU buffer specialized for vertex data.
 */
class VertexBuffer : public GpuBuffer
{
public:
  uint64 getVertexSizeBytes() const { return _vertexSizeBytes; }
  uint32 getVertexCount() const { return _vertexCount; }

protected:
  VertexBuffer(const VertexBufferDesc &desc) : GpuBuffer({desc.VertexSizeBytes * desc.VertexCount,
                                                          BufferType::Vertex,
                                                          desc.BufferUsage}),
                                               _vertexSizeBytes(desc.VertexSizeBytes),
                                               _vertexCount(desc.VertexCount)
  {
  }

private:
  uint64 _vertexSizeBytes;
  uint32 _vertexCount;
};

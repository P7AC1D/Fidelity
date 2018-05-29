#pragma once
#include "GpuBuffer.hpp"

enum class IndexType
{
  Int16,
  Int32
};

struct IndexBufferDesc
{
  uint32 IndexCount;
  IndexType IndexType = IndexType::Int32;
  BufferUsage BufferUsage = BufferUsage::Default;
};

uint32 GetIndexTypeSize(IndexType type)
{
  switch (type)
  {
    case IndexType::Int16:
      return 2;
    case IndexType::Int32:
    default:
      return 4;
  }
}

class IndexBuffer : public GpuBuffer
{
public:
  uint32 GetIndexCount() const { return _desc.IndexCount; }
  IndexType GetIndexType() const { return _desc.IndexType; }
  BufferUsage GetBufferUsage() const { return _desc.BufferUsage; }
  
protected:
  IndexBuffer(const IndexBufferDesc& desc):
    GpuBuffer({
      GetIndexTypeSize(desc.IndexType) * desc.IndexCount,
      BufferType::Index,
      desc.BufferUsage
    }),
    _desc(desc)
  {}
  
protected:
  IndexBufferDesc _desc;
};

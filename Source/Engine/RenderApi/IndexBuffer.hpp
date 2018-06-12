#pragma once
#include "GpuBuffer.hpp"

enum class IndexType
{
  UInt16,
  UInt32
};

struct IndexBufferDesc
{
  uint32 IndexCount;
  IndexType IndexType = IndexType::UInt32;
  BufferUsage BufferUsage = BufferUsage::Default;
};

class IndexBuffer : public GpuBuffer
{
public:
  static uint32 GetBytesPerIndex(IndexType type)
  {
    switch (type)
    {
      case IndexType::UInt16:
        return 2;
      case IndexType::UInt32:
      default:
        return 4;
    }
  }
  
  uint32 GetIndexCount() const { return _desc.IndexCount; }
  IndexType GetIndexType() const { return _desc.IndexType; }
  BufferUsage GetBufferUsage() const { return _desc.BufferUsage; }
  
protected:
  IndexBuffer(const IndexBufferDesc& desc):
    GpuBuffer({
      GetBytesPerIndex(desc.IndexType) * desc.IndexCount,
      BufferType::Index,
      desc.BufferUsage
    }),
    _desc(desc)
  {}
  
protected:
  IndexBufferDesc _desc;
};

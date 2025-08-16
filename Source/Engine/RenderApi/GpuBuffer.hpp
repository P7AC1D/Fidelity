#pragma once
#include "../Core/Types.hpp"
#include "ResourceHandle.hpp"
#include <stdexcept>

// Usage and memory hints for cross-API mapping

// Buffer usage flags (bitmask)
enum class GpuBufferUsage : uint32
{
  None = 0,
  Vertex = 1u << 0,
  Index = 1u << 1,
  Uniform = 1u << 2,
  Storage = 1u << 3,
  Indirect = 1u << 4,
  TransferSrc = 1u << 5,
  TransferDst = 1u << 6,
};

inline constexpr GpuBufferUsage operator|(GpuBufferUsage a, GpuBufferUsage b)
{
  return static_cast<GpuBufferUsage>(static_cast<uint32>(a) | static_cast<uint32>(b));
}
inline constexpr GpuBufferUsage &operator|=(GpuBufferUsage &a, GpuBufferUsage b)
{
  a = a | b;
  return a;
}
inline constexpr bool operator&(GpuBufferUsage a, GpuBufferUsage b)
{
  return (static_cast<uint32>(a) & static_cast<uint32>(b)) != 0u;
}

// Coarse memory placement hint
enum class MemoryUsage : uint8
{
  GpuOnly,  // device-local; staging required for CPU writes
  CpuToGpu, // CPU-visible write-combined
  GpuToCpu, // CPU-visible read-back
};

enum CpuAccess
{
  CPU_ACCESS_WRITE = 0x10000,
  CPU_ACCESS_READ = 0x20000
};

enum ResourceMiscFlag
{
  RESOURCE_MISC_GENERATE_MIPS = 0x1,
  RESOURCE_MISC_SHARED = 0x2,
  RESOURCE_MISC_TEXTURED_CUBE = 0x4,
  RESOURCE_MISC_DRAW_INDEIRECT = 0x10,
  RESOURCE_MISC_BUFFER_ALLOW_RAW_VALUES = 0x10,
  RESOURCE_MISC_BUFFER_STRUCTERED = 0x20,
  RESOURCE_MISC_TILED = 0x40000
};

enum class BufferUsage
{
  Default,
  Dynamic,
  Stream
};

enum class BufferType
{
  Vertex,
  Index,
  Constant,
};

enum class AccessType
{
  ReadOnly,
  WriteOnly,
  ReadWrite,
  WriteOnlyDiscard,
  WriteOnlyDiscardRange,
  WriteOnlyUnsynchronized,
};

struct GpuBufferDesc
{
  uint64 ByteCount;
  BufferType BufferType;
  BufferUsage BufferUsage = BufferUsage::Default;
  uint32 CpuAccessFlags = 0;
  uint32 ResourceMiscFlags = 0;
  // Additional cross-API hints (optional; legacy fields above remain for compatibility)
  GpuBufferUsage UsageFlags = GpuBufferUsage::None;
  MemoryUsage Memory = MemoryUsage::GpuOnly;
};

class GpuBuffer : public ResourceHandle
{
public:
  const GpuBufferDesc &getDesc() const { return _desc; }
  BufferType getType() const { return _desc.BufferType; }
  uint64 getSizeBytes() const { return _desc.ByteCount; }
  bool isInitialized() const { return _initialized; }

  // Explicit map/unmap. Default throws if backend doesn't support mapping.
  // Prefer using map/unmap for staging uploads; writeData/readData remain as helpers.
  virtual void *map(uint64 byteOffset, uint64 byteCount, AccessType accessType = AccessType::ReadWrite)
  {
    (void)byteOffset;
    (void)byteCount;
    (void)accessType;
    throw std::runtime_error("GpuBuffer::map not supported by this backend");
  }
  virtual void unmap() { throw std::runtime_error("GpuBuffer::unmap not supported by this backend"); }

  virtual void writeData(uint64 byteOffset, uint64 byteCount, const void *src, AccessType accessType = AccessType::WriteOnly) = 0;
  virtual void readData(uint64 byteOffset, uint64 byteCount, void *dst) = 0;
  virtual void copyData(GpuBuffer *dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount) = 0;

protected:
  GpuBuffer(const GpuBufferDesc &desc) : _desc(desc), _initialized(false) {}

protected:
  GpuBufferDesc _desc;
  bool _initialized;
};

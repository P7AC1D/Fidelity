#pragma once
#include "../Core/Types.hpp"

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
};

class GpuBuffer
{
public:
  const GpuBufferDesc &getDesc() const { return _desc; }
  BufferType getType() const { return _desc.BufferType; }
  uint64 getSizeBytes() const { return _desc.ByteCount; }
  bool isInitialized() const { return _initialized; }

  virtual void writeData(uint64 byteOffset, uint64 byteCount, const void *src, AccessType accessType = AccessType::WriteOnly) = 0;
  virtual void readData(uint64 byteOffset, uint64 byteCount, void *dst) = 0;
  virtual void copyData(GpuBuffer *dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount) = 0;

protected:
  GpuBuffer(const GpuBufferDesc &desc) : _desc(desc), _initialized(false) {}

protected:
  GpuBufferDesc _desc;
  bool _initialized;
};

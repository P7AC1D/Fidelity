#pragma once
#include "../Core/Types.hpp"

enum BindFlag
{
  BIND_FLAG_VERTEX_BUFFER = 0x1L,
  BIND_FLAG_INDEX_BUFFER = 0x2L,
  BIND_FLAG_CONSTANT_BUFFER = 0x4L,
  BIND_FLAG_SHADER_RESOURCE = 0x8L,
  BIND_FLAG_STREAM_OUTPUT = 0x10L,
  BIND_FLAG_RENDER_TARGET = 0x20L,
  BIND_FLAG_DEPTH_STENCIL = 0x40L,
  BIND_FLAG_UNORDERED_ACCESS = 0x80L
};

enum CpuAccess
{
  CPU_ACCESS_WRITE = 0x10000L,
  CPU_ACCESS_READ = 0x20000L
};

enum ResourceMiscFlag
{
  RESOURCE_MISC_GENERATE_MIPS = 0x1L,
  RESOURCE_MISC_SHARED = 0x2L,
  RESOURCE_MISC_TEXTURED_CUBE = 0x4L,
  RESOURCE_MISC_DRAW_INDEIRECT = 0x10L,
  RESOURCE_MISC_BUFFER_ALLOW_RAW_VALUES = 0x10L,
  RESOURCE_MISC_BUFFER_STRUCTERED = 0x20L,
  RESOURCE_MISC_TILED = 0x40000L
};

enum class BufferUsage
{
  Default,
  Immutable,
  Dynamic,
  Staging,
};

struct GpuBufferDesc
{
  uint32 ByteCount;
  uint32 BindFlags;
  BufferUsage BufferUsage = BufferUsage::Default;
  uint32 CpuAccessFlags = 0;
  uint32 ResourceMiscFlags = 0;
};

class GpuBuffer
{
public:
  const GpuBufferDesc& GetDesc() const { return _desc; }
  
protected:
  GpuBufferDesc _desc;
};

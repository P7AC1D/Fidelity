#pragma once
#include "../GpuBuffer.hpp"

class GLGpuBuffer : public GpuBuffer
{
  friend class GLIndexBuffer;
  friend class GLRenderDevice;
  friend class GLVertexBuffer;

public:
  virtual ~GLGpuBuffer();

  uint32 GetId() const { return _id; }

  void WriteData(uint64 byteOffset, uint64 byteCount, const void* src, AccessType accessType = AccessType::WriteOnly) override;
  void ReadData(uint64 byteOffset, uint64 byteCount, void* dst) override;
  void CopyData(GpuBuffer* dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount) override;
  
protected:
  GLGpuBuffer(const GpuBufferDesc& desc);
  
private:
  void Initialize();
  void* MapRange(uint64 byteOffset, uint64 byteCount, AccessType accessType);
  void Unmap();
  
private:
  uint32 _id;
};

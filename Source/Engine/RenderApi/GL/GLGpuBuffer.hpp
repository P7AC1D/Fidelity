#pragma once
#include "../GpuBuffer.hpp"

class GLGpuBuffer : public GpuBuffer
{
  friend class GLIndexBuffer;
  friend class GLRenderDevice;
  friend class GLVertexBuffer;

public:
  virtual ~GLGpuBuffer();

  void *getNativeHandle() const override;
  bool isValid() const override;

  uint32 GetId() const;

  void writeData(uint64 byteOffset, uint64 byteCount, const void *src, AccessType accessType = AccessType::WriteOnly) override;
  void readData(uint64 byteOffset, uint64 byteCount, void *dst) override;
  void copyData(GpuBuffer *dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount) override;

  // Phase 5: explicit mapping
  void *map(uint64 byteOffset, uint64 byteCount, AccessType accessType = AccessType::ReadWrite) override { return MapRange(byteOffset, byteCount, accessType); }
  void unmap() override { Unmap(); }

protected:
  GLGpuBuffer(const GpuBufferDesc &desc);

private:
  void Initialize();
  void *MapRange(uint64 byteOffset, uint64 byteCount, AccessType accessType);
  void Unmap();

private:
  uint32 _id;
};

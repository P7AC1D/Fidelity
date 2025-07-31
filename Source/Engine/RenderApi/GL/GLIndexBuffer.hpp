#pragma once
#include <memory>
#include "../IndexBuffer.hpp"

class GLGpuBuffer;

class GLIndexBuffer : public IndexBuffer
{
  friend class GLRenderDevice;

public:
  GLIndexBuffer(const IndexBufferDesc &desc);

  void *getNativeHandle() const override;
  bool isValid() const override;

  void writeData(uint64 byteOffset, uint64 byteCount, const void *src, AccessType accessType = AccessType::WriteOnly) override;
  void readData(uint64 byteOffset, uint64 byteCount, void *dst) override;
  void copyData(GpuBuffer *dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount) override;

  uint32 getId() const;

private:
  std::unique_ptr<GLGpuBuffer> _buffer;
};

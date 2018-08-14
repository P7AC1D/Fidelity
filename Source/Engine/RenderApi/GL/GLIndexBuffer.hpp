#pragma once
#include <memory>
#include "../IndexBuffer.hpp"

class GLGpuBuffer;

class GLIndexBuffer: public IndexBuffer
{
  friend class GLRenderDevice;
  
public:
  void WriteData(uint64 byteOffset, uint64 byteCount, const void* src, AccessType accessType = AccessType::WriteOnly) override;
  void ReadData(uint64 byteOffset, uint64 byteCount, void* dst) override;
  void CopyData(GpuBuffer* dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount) override;
  
  uint32 GetId() const;
  
protected:
  GLIndexBuffer(const IndexBufferDesc& desc);
  
private:
  std::unique_ptr<GLGpuBuffer> _buffer;
};

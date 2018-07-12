#pragma once
#include <memory>
#include "../VertexBuffer.hpp"

class GLGpuBuffer;
class GLVertexArrayObject;

class GLVertexBuffer : public VertexBuffer
{
  friend class GLRenderDevice;
  friend class GLVertexArrayObjectCollection;
  
public:
  uint32 GetId() const;
  
  void WriteData(uint64 byteOffset, uint64 byteCount, const void* src, AccessType accessType = AccessType::WriteOnly) override;
  void ReadData(uint64 byteOffset, uint64 byteCount, void* dst) override;
  void CopyData(GpuBuffer* dst, uint64 srcByteOffset, uint64 dstByteOffset, uint64 byteCount) override;
  
protected:
  GLVertexBuffer(const VertexBufferDesc& desc);
  
private:
  std::unique_ptr<GLGpuBuffer> _buffer;
  std::shared_ptr<GLVertexArrayObject> _vao;
};

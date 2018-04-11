#pragma once
#include "GpuBuffer.hpp"

class GLGpuBuffer : public GpuBuffer
{
public:
  uint32 GetId() const { return _id; }
  
protected:
  GLGpuBuffer(const GpuBufferDesc& desc);
  
private:
  void Initialize();
  
private:
  uint32 _id;
};

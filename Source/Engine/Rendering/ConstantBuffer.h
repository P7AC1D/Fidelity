#pragma once
#include "../Core/Types.hpp"

namespace Rendering
{
class ConstantBuffer
{
public:
  ConstantBuffer(int32 sizeBytes);
  ~ConstantBuffer();

  void AllocateMemory(int32 sizeBytes);
  void UploadData(int32 offsetBytes, int32 countBytes, const void* data);

private:
  ConstantBuffer(ConstantBuffer&) = delete;
  ConstantBuffer(ConstantBuffer&&) = delete;
  ConstantBuffer& operator=(ConstantBuffer&) = delete;
  ConstantBuffer& operator=(ConstantBuffer&&) = delete;

  inline void Initialize();

  uint32 _uboId;
  int32 _sizeBytes;
  bool _isInitialized;

  friend class Renderer;
};
}

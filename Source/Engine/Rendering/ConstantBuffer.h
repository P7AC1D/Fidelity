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

  inline uint32 GetId() const { return _uboId; }

private:
  inline void Initialize();

  uint32 _uboId;
  int32 _sizeBytes;
  bool _isInitialized;
};
}

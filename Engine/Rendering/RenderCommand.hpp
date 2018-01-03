#pragma once
#include "Types.h"

class RenderCommand
{
public:
  RenderCommand(uint32 gpuProgramId, uint32 materialId, uint32 depth,
                bool isTransparent, uint32 vboId, uint32 iboId, uint32 fboId);
  
  bool operator<(const RenderCommand& rhs) const;
  bool operator==(const RenderCommand& rhs) const;
  
  inline uint64 GetKey() const { return _key; }
  
private:
  uint64 _key;
  uint32 _vboId;
  uint32 _iboId;
  uint32 _fboId;
};

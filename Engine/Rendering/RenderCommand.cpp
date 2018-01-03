#include "RenderCommand.hpp"

#include <cassert>

RenderCommand::RenderCommand(uint32 gpuProgramId, uint32 materialId, uint32 depth,
                             bool isTransparent, uint32 vboId, uint32 iboId, uint32 fboId):
_key(0),
_vboId(vboId),
_iboId(iboId),
_fboId(fboId)
{
  assert(gpuProgramId < 4095);
  assert(materialId < 16777215);
  assert(depth < 16777215);
  _key += ((isTransparent ? 0x00000001 : 0x00000000) & 0x0000000F) << 0;
  _key += (gpuProgramId & 0x00000FFF) << 4;
  _key += (materialId & 0x00FFFFFF) << 16;
  _key += (uint64)(depth & 0x00FFFFFF) << 40;
}

bool RenderCommand::operator<(const RenderCommand& rhs) const
{
  // Test transparency
  if ((_key & 0x000000000000000F) < (rhs._key & 0x000000000000000F))
  {
    return true;
  }
  if ((_key & 0x000000000000000F) > (rhs._key & 0x000000000000000F))
  {
    return false;
  }
  
  bool isTransparent = (bool)(_key & 0x000000000000000F);
  if (isTransparent)
  {
    // Test depth
    if ((_key & 0xFFFFFF0000000000) < (rhs._key & 0xFFFFFF0000000000))
    {
      return true;
    }
    if ((_key & 0xFFFFFF0000000000) > (rhs._key & 0xFFFFFF0000000000))
    {
      return false;
    }
    
    // Test gpu program ID
    if ((_key & 0x000000000000FFF0) < (rhs._key & 0x000000000000FFF0))
    {
      return true;
    }
    if ((_key & 0x000000000000FFF0) > (rhs._key & 0x000000000000FFF0))
    {
      return false;
    }
    
    // Test material ID
    if ((_key & 0x000000FFFFFF0000) < (rhs._key & 0x000000FFFFFF0000))
    {
      return true;
    }
  }
  else
  {
    // Test gpu program ID
    if ((_key & 0x000000000000FFF0) < (rhs._key & 0x000000000000FFF0))
    {
      return true;
    }
    if ((_key & 0x000000000000FFF0) > (rhs._key & 0x000000000000FFF0))
    {
      return false;
    }
    
    // Test material ID
    if ((_key & 0x000000FFFFFF0000) < (rhs._key & 0x000000FFFFFF0000))
    {
      return true;
    }
    if ((_key & 0x000000FFFFFF0000) > (rhs._key & 0x000000FFFFFF0000))
    {
      return false;
    }
    
    // Test depth
    if ((_key & 0xFFFFFF0000000000) < (rhs._key & 0xFFFFFF0000000000))
    {
      return true;
    }
  }
  return false;
}

bool RenderCommand::operator==(const RenderCommand& rhs) const
{
  return this->_key == rhs._key;
  
}

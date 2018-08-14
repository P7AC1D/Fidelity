#pragma once
#include "../RenderTarget.hpp"

class GLRenderTarget : public RenderTarget
{
  friend class GLRenderDevice;
  
public:
  virtual ~GLRenderTarget();
  
  uint32 GetId() const { return _id; }
  
protected:
  GLRenderTarget(const RenderTargetDesc& desc);
  
private:
  void Initialize();
  
private:
  uint32 _id;
};

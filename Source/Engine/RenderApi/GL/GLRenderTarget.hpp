#pragma once
#include "../RenderTarget.hpp"

class GLRenderTarget : public RenderTarget
{
  friend class GLRenderDevice;

public:
  virtual ~GLRenderTarget();

  void copy(const std::shared_ptr<RenderTarget> &target) override;

  uint32 getId() const { return _id; }

protected:
  GLRenderTarget(const RenderTargetDesc &desc);

private:
  void initialize();

private:
  uint32 _id;
};

#include "RenderTarget.hpp"

#include <stdexcept>
#include <vector>

#include "OpenGL.h"
#include "Texture.h"

static uint32 ActiveRenderTarget = -1;

namespace Rendering {
  RenderTarget::RenderTarget(RenderTargetDesc desc):
    _desc(desc)
  {
    GLCall(glGenFramebuffers(1, &_fbo));
    Initialize();
  }

  RenderTarget::~RenderTarget()
  {
    Disable();
    GLCall(glDeleteFramebuffers(1, &_fbo));
  }

  std::shared_ptr<Texture> RenderTarget::GetColourBuffer(uint32 index) const
  {
    if (index > MaxColourBuffers)
    {
      throw std::runtime_error("Colour buffer index larger than the maximum allowed colour buffers attached to a render target.");
    }

    return _colourBuffers[index];
  }

  void RenderTarget::Enable() const
  {
    if (ActiveRenderTarget != _fbo)
    {
      GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));
      ActiveRenderTarget = _fbo;
    }
  }

  void RenderTarget::Disable() const
  {
    if (ActiveRenderTarget == _fbo)
    {
      GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
      ActiveRenderTarget = -1;
    }
  }

  void RenderTarget::Initialize()
  {
    Enable();

    std::vector<GLenum> attachments;
    for (uint32 i = 0; i < _desc.ColourBufferCount; i++)
    {
      TextureDesc textureDesc;
      textureDesc.Width = _desc.Width;
      textureDesc.Height = _desc.Height;
      textureDesc.Format = PixelFormat::RGB16F;
      _colourBuffers[i].reset(new Texture(textureDesc));
      GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _colourBuffers[i]->_id, 0));
      attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    if (_desc.EnableStencilBuffer)
    {
      TextureDesc textureDesc;
      textureDesc.Width = _desc.Width;
      textureDesc.Height = _desc.Height;
      textureDesc.Format = PixelFormat::D24S8;
      _depthStencilBuffer.reset(new Texture(textureDesc));
      GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _depthStencilBuffer->_id, 0));
      GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthStencilBuffer->_id, 0));
    }
    else
    {
      TextureDesc textureDesc;
      textureDesc.Width = _desc.Width;
      textureDesc.Height = _desc.Height;
      textureDesc.Format = PixelFormat::D32;
      _depthStencilBuffer.reset(new Texture(textureDesc));
      GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthStencilBuffer->_id, 0));
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      throw std::runtime_error("RenderTarget intialization failure.");
    }

    if (_desc.ColourBufferCount > 0)
    {
      GLCall(glDrawBuffers(3, attachments.data()));
    }
    else
    {
      GLCall(glDrawBuffer(GL_NONE));
    }

    Disable();
  }
}
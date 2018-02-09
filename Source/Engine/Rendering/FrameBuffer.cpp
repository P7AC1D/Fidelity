#include "FrameBuffer.h"

#include <vector>

#include "OpenGL.h"
#include "Texture.h"

static uint32 ActiveFrameBuffer = -1;

namespace Rendering
{
FrameBuffer::FrameBuffer(uint32 width, uint32 height, uint32 target):
  _width(width),
  _height(height)
{
  GLCall(glGenFramebuffers(1, &_fbo));
  Bind();
  
  std::vector<GLenum> attachments;
  uint32 colourBufferCount = 0;
  if (target & FrameBufferTarget::FBT_Colour0)
  {
    TextureDesc desc;
    desc.Width = _width;
    desc.Height = _height;
    desc.Format = PixelFormat::RGB16F;
    _colourTexture0.reset(new Texture(desc));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colourTexture0->_id, 0));
    colourBufferCount++;
    attachments.push_back(GL_COLOR_ATTACHMENT0);
  }
  if (target & FrameBufferTarget::FBT_Colour1)
  {
    TextureDesc desc;
    desc.Width = _width;
    desc.Height = _height;
    desc.Format = PixelFormat::RGB16F;
    _colourTexture1.reset(new Texture(desc));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _colourTexture1->_id, 0));
    colourBufferCount++;
    attachments.push_back(GL_COLOR_ATTACHMENT1);
  }
  if (target & FrameBufferTarget::FBT_Colour2)
  {
    TextureDesc desc;
    desc.Width = _width;
    desc.Height = _height;
    desc.Format = PixelFormat::RGB16F;
    _colourTexture2.reset(new Texture(desc));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _colourTexture2->_id, 0));
    colourBufferCount++;
    attachments.push_back(GL_COLOR_ATTACHMENT2);
  }
  if (target & FrameBufferTarget::FBT_Depth)
  {
    TextureDesc desc;
    desc.Width = _width;
    desc.Height = _height;
    desc.Format = PixelFormat::D32;
    desc.WrapMethod = WrapMethod::ClampToEdge;
    _depthTexture.reset(new Texture(desc));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture->_id, 0));
  }
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    throw std::runtime_error("FrameBuffer construction failure.");
  }
  
  GLCall(glReadBuffer(GL_NONE));
  if (colourBufferCount == 0)
  {
    GLCall(glDrawBuffer(GL_NONE));
  }
  else
  {
    GLCall(glDrawBuffers(3, attachments.data()));
  }

  Unbind();
}
  
FrameBuffer::~FrameBuffer()
{
  GLCall(glDeleteFramebuffers(1, &_fbo));
}

void FrameBuffer::Bind() const
{
  if (ActiveFrameBuffer != _fbo)
  {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));
    ActiveFrameBuffer = _fbo;
  }
}
  
void FrameBuffer::Unbind() const
{
  if (ActiveFrameBuffer == _fbo)
  {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    ActiveFrameBuffer = -1;
  }
}
}

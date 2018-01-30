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
    _colourTexture0.reset(new Texture(PixelFormat::RGB16F, _width, _height));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colourTexture0->_id, 0));
    colourBufferCount++;
    attachments.push_back(GL_COLOR_ATTACHMENT0);
  }
  if (target & FrameBufferTarget::FBT_Colour1)
  {
    _colourTexture1.reset(new Texture(PixelFormat::RGB16F, _width, _height));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _colourTexture1->_id, 0));
    colourBufferCount++;
    attachments.push_back(GL_COLOR_ATTACHMENT1);
  }
  if (target & FrameBufferTarget::FBT_Colour2)
  {
    _colourTexture2.reset(new Texture(PixelFormat::RGBA, _width, _height));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _colourTexture2->_id, 0));
    colourBufferCount++;
    attachments.push_back(GL_COLOR_ATTACHMENT2);
  }
  if (target & FrameBufferTarget::FBT_Depth)
  {
    _depthTexture.reset(new Texture(PixelFormat::Depth, _width, _height));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture->_id, 0));

    _depthTexture->SetWrapMethod(TextureWrapMethod::ClampToEdge);
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
  GLCall(glDrawBuffer(GL_BACK));
}
}

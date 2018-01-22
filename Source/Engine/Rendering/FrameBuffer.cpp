#include "FrameBuffer.h"

#include "Texture.h"

static uint32 ActiveFrameBuffer = -1;

namespace Rendering
{
FrameBuffer::FrameBuffer(uint32 width, uint32 height, FrameBufferTarget target):
  _width(width),
  _height(height),
  _target(target)
{
  glGenFramebuffers(1, &_fbo);
  Bind();
  
  uint32 colourBufferCount = 0;
  if (target & FrameBufferTarget::FBT_Colour0)
  {
    _colourTexture0.reset(new Texture(TextureFormat::RGBA, _height, _width));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colourTexture0->_id, 0);
    colourBufferCount++;
  }
  if (target & FrameBufferTarget::FBT_Colour1)
  {
    _colourTexture1.reset(new Texture(TextureFormat::RGBA, _height, _width));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _colourTexture0->_id, 0);
    colourBufferCount++;
  }
  if (target & FrameBufferTarget::FBT_Colour2)
  {
    _colourTexture2.reset(new Texture(TextureFormat::RGBA, _height, _width));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _colourTexture0->_id, 0);
    colourBufferCount++;
  }
  if (target & FrameBufferTarget::FBT_Depth)
  {
    _depthTexture.reset(new Texture(TextureFormat::Depth, _height, _width));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture->_id, 0);
    if (colourBufferCount == 0)
    {
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
    }
  }
  Unbind();
}
  
FrameBuffer::~FrameBuffer()
{
  glDeleteFramebuffers(1, &_fbo);
}

void FrameBuffer::Bind() const
{
  if (ActiveFrameBuffer != _fbo)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  }
}
  
void FrameBuffer::Unbind() const
{
  if (ActiveFrameBuffer == _fbo)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}
}
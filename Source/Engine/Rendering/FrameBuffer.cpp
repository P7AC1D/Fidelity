#include "FrameBuffer.h"

#include <sstream>

#include "OpenGL.h"
#include "Texture.h"

namespace Rendering
{
int32 FrameBuffer::CurrentlyBoundFboId = 0;

FrameBuffer::FrameBuffer()
{
  GLCall(glGenFramebuffers(1, &_fboId));
}

FrameBuffer::~FrameBuffer()
{
  Unbind();
  GLCall(glDeleteFramebuffers(1, &_fboId));
}

void FrameBuffer::Activate()
{
  RetrieveViewportDimensions();

  Bind();
  glViewport(0, 0, _colourTexture->GetWidth(), _colourTexture->GetHeight());

  auto result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (result != GL_FRAMEBUFFER_COMPLETE)
  {
    std::stringstream errorMesage;
    switch (result)
    {
    case GL_FRAMEBUFFER_UNDEFINED:
      errorMesage << "FBO " << _fboId << " GL_FRAMEBUFFER_UNDEFINED error: the specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      errorMesage << "FBO " << _fboId << " GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT error";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      errorMesage << "FBO " << _fboId << " GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT error";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      errorMesage << "FBO " << _fboId << " GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER error";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      errorMesage << "FBO " << _fboId << " GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER error";
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      errorMesage << "FBO " << _fboId << " GL_FRAMEBUFFER_UNSUPPORTED error";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      errorMesage << "FBO " << _fboId << " GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE error";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      errorMesage << "FBO " << _fboId << " GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS error";
      break;
    default:
      errorMesage << "FBO " << _fboId << " encountered an unknown error";
      break;
    }
    throw std::runtime_error(errorMesage.str());
  }
}

void FrameBuffer::Deactivate() const
{
  GLCall(glViewport(0, 0, _prevViewportWidth, _prevViewportHeight));
  Unbind();
}

bool FrameBuffer::IsBound() const
{
  return CurrentlyBoundFboId == _fboId;
}

void FrameBuffer::SetColourTexture(std::shared_ptr<Texture> texture)
{
  _colourTexture = texture;
  Bind();
  GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colourTexture->_id, 0));
}

void FrameBuffer::SetDepthTexture(std::shared_ptr<Texture> texture)
{
  _depthTexture = texture;
  Bind();
  GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture->_id, 0));
}

void FrameBuffer::RetrieveViewportDimensions()
{
  int32 result[4];
  GLCall(glGetIntegerv(GL_VIEWPORT, result));
  _prevViewportWidth = result[2];
  _prevViewportHeight = result[3];
}

void FrameBuffer::Bind() const
{
  if (!IsBound())
  {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _fboId));
    CurrentlyBoundFboId = _fboId;
  }
}

void FrameBuffer::Unbind() const
{
  if (IsBound())
  {
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    CurrentlyBoundFboId = 0;
  }
}
}

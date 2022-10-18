#include "GLRenderTarget.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"
#include "GLTexture.hpp"

void attachDepthStencilTexture(const std::shared_ptr<Texture> &texture)
{
  auto glTexture = std::dynamic_pointer_cast<GLTexture>(texture);
  const auto &textureDesc = glTexture->getDesc();
  if (textureDesc.Usage == TextureUsage::Depth)
  {
    glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glTexture->getId(), 0));
  }
  else if (textureDesc.Usage == TextureUsage::DepthStencil)
  {
    glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, glTexture->getId(), 0));
  }
}

void attachColourTexture(const std::shared_ptr<Texture> &texture, uint32 attachmentIndex)
{
  auto glTexture = std::dynamic_pointer_cast<GLTexture>(texture);
  glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, glTexture->getId(), 0));
  glCall(glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex));
}

GLRenderTarget::~GLRenderTarget()
{
  if (_id != 0)
  {
    glCall(glDeleteFramebuffers(1, &_id));
  }
}

GLRenderTarget::GLRenderTarget(const RenderTargetDesc &desc) : RenderTarget(desc), _id(0)
{
  initialize();
}

void GLRenderTarget::initialize()
{
  if (isInitialized())
  {
    return;
  }

  glCall(glGenFramebuffers(1, &_id));
  ASSERT_FALSE(_id == 0, "Could not generate frame buffer object");

  std::vector<GLenum> attachments;
  glCall(glBindFramebuffer(GL_FRAMEBUFFER, _id));
  for (uint32 i = 0; i < MaxColourTargets; i++)
  {
    if (_desc.ColourTargets[i] != nullptr)
    {
      attachColourTexture(_desc.ColourTargets[i], i);
      attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
  }

  if (_desc.DepthStencilTarget != nullptr)
  {
    attachDepthStencilTexture(_desc.DepthStencilTarget);
  }

  if (attachments.size() > 0)
  {
    glCall(glDrawBuffers(attachments.size(), attachments.data()));
  }
  else
  {
    glCall(glDrawBuffer(GL_NONE));
    glCall(glReadBuffer(GL_NONE));
  }

  GLenum frameBufferComplete = 0;
  glCall2(glCheckFramebufferStatus(GL_FRAMEBUFFER), frameBufferComplete);
  ASSERT_TRUE(frameBufferComplete == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete");

  glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

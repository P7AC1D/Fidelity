#include "GLRenderTarget.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"
#include "GLTexture.hpp"

void AttachDepthStencilTexture(const std::shared_ptr<Texture>& texture)
{
  auto glTexture = std::dynamic_pointer_cast<GLTexture>(texture);
  const auto& textureDesc = glTexture->GetDesc(); 
  if (textureDesc.Usage == TextureUsage::Depth)
  {
    GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glTexture->GetId(), 0));
  }
  else if (textureDesc.Usage == TextureUsage::DepthStencil)
  {
    GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, glTexture->GetId(), 0));
  }
}

void AttachColourTexture(const std::shared_ptr<Texture>& texture, uint32 attachmentIndex)
{
  auto glTexture = std::dynamic_pointer_cast<GLTexture>(texture);
  GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, glTexture->GetId(), 0));
  GLCall(glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex));
}

GLRenderTarget::~GLRenderTarget()
{
  if (_id != 0)
  {
    GLCall(glDeleteFramebuffers(1, &_id));
  }
}

GLRenderTarget::GLRenderTarget(const RenderTargetDesc& desc): RenderTarget(desc), _id(0)
{
  Initialize();
}

void GLRenderTarget::Initialize()
{
  if (IsInitialized())
  {
    return;
  }
  
  GLCall(glGenFramebuffers(1, &_id));
  ASSERT_FALSE(_id == 0, "Could not generate frame buffer object");
  
  std::vector<GLenum> attachments;
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, _id));
  for (uint32 i = 0; i < MaxColourTargets; i++)
  {
    if (_desc.ColourTargets[i] != nullptr)
    {
      AttachColourTexture(_desc.ColourTargets[i], i);
      attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
  }
  
  if (_desc.DepthStencilTarget != nullptr)
  {
    AttachDepthStencilTexture(_desc.DepthStencilTarget);
  }

  if (attachments.size() > 0)
  {
    GLCall(glDrawBuffers(attachments.size(), attachments.data()));
  }
  else
  {
    GLCall(glDrawBuffer(GL_NONE));
    GLCall(glReadBuffer(GL_NONE));
  }

  GLenum frameBufferComplete = 0;
	GLCall2(glCheckFramebufferStatus(GL_FRAMEBUFFER), frameBufferComplete);
  ASSERT_TRUE(frameBufferComplete == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete");
  
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

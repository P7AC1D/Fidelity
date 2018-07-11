#include "GLRenderTarget.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"
#include "GLTexture.hpp"

void AttachDepthStencilTexture(const std::shared_ptr<Texture>& texture)
{
  auto glTexture = std::dynamic_pointer_cast<GLTexture>(texture);
  auto textureDesc = glTexture->GetDesc();
  switch (textureDesc.Type)
  {
    case TextureType::Texture2D:
      GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glTexture->GetId(), 0));
      GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTexture->GetId(), 0));
      break;
    default:
      throw std::runtime_error("Unsupported TextureType for depth-stencil target attachment");
  }
}

void AttachColourTexture(const std::shared_ptr<Texture>& texture, uint32 attachmentIndex)
{
  auto glTexture = std::dynamic_pointer_cast<GLTexture>(texture);
  auto textureDesc = glTexture->GetDesc();
  switch (textureDesc.Type)
  {
    case TextureType::Texture1D:
      GLCall(glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_TEXTURE_1D, glTexture->GetId(), 0));
      break;
    case TextureType::Texture2D:
      GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_TEXTURE_2D, glTexture->GetId(), 0));
      break;
    case TextureType::TextureCube:
      GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, glTexture->GetId(), 0));
      break;
    case TextureType::Texture1DArray:
    case TextureType::Texture2DArray:
    case TextureType::Texture3D:
    default:
      throw std::runtime_error("Unsupported TextureType for colour target attachment");
  }
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
  GLCall(glDrawBuffers(attachments.size(), attachments.data()));

  GLenum frameBufferComplete = 0;
	GLCall2(glCheckFramebufferStatus(GL_FRAMEBUFFER), frameBufferComplete);
  ASSERT_TRUE(frameBufferComplete == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete");
}

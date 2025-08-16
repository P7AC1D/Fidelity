#include "GLCommandBuffer.hpp"
#include "GLRenderDevice.hpp"
#include "../../Utility/Assert.hpp"
#include "../RenderDevice.hpp"
#include "../PrimitiveTopology.hpp"
#include "../VertexBuffer.hpp"
#include "../IndexBuffer.hpp"
#include "../Texture.hpp"
#include "../GpuBuffer.hpp"
#include "../ResourceSet.hpp"
#include "../Shader.hpp"
#include "../ShaderParams.hpp"
#include "GLShader.hpp"
#include "GLCommon.hpp"
#include "GLResourceSet.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLShaderPipeline.hpp"
#include "GLShaderPipelineCollection.hpp"
#include "GLVertexArrayCollection.hpp"
#include "../../Maths/Colour.hpp"
#include "../RenderPass.hpp"
#include "../Framebuffer.hpp"
#include "../PipelineLayout.hpp"
#include "../PushConstants.hpp"
#include "../RenderLimits.hpp"
#include "../GraphicsPipelineState.hpp"
#include "../ComputePipelineState.hpp"
// Needed for buffer/texture copy implementations
#include "GLTexture.hpp"
#include "GLGpuBuffer.hpp"
// Memory barrier helpers
#include "GLCommon.hpp"
#include "../Query.hpp"

// Local helpers for better diagnostics
namespace
{
  const char *framebufferStatusToString(GLenum status)
  {
    switch (status)
    {
    case GL_FRAMEBUFFER_COMPLETE:
      return "GL_FRAMEBUFFER_COMPLETE";
    case GL_FRAMEBUFFER_UNDEFINED:
      return "GL_FRAMEBUFFER_UNDEFINED";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
#endif
    case GL_FRAMEBUFFER_UNSUPPORTED:
      return "GL_FRAMEBUFFER_UNSUPPORTED";
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
#endif
    default:
      return "GL_FRAMEBUFFER_UNKNOWN_STATUS";
    }
  }
}

GLCommandBuffer::GLCommandBuffer(std::shared_ptr<GLRenderDevice> device)
    : _device(device),
      _state(CommandBufferState::Initial),
      _usage(CommandBufferUsage::OneTimeSubmit),
      _currentFboId(0),
      _ownsCurrentFbo(false),
      _primitiveTopology(PrimitiveTopology::TriangleList),
      _boundIndexBuffer(nullptr),
      _boundVertexBuffer(nullptr),
      _shaderPipeline(nullptr),
      _shaderPipelineCollection(nullptr),
      _hasActivePipeline(false),
      _hasActiveVertexBuffer(false),
      _hasActiveIndexBuffer(false)
{
  ASSERT_FALSE(_device == nullptr, "GLCommandBuffer: Device cannot be null");

  // Get shader pipeline collection from device for pipeline management
  _shaderPipelineCollection = _device->_shaderPipelineCollection;
}

void GLCommandBuffer::begin(CommandBufferUsage usage)
{
  validateState(CommandBufferState::Initial, "begin");

  _usage = usage;
  _state = CommandBufferState::Recording;

  // Reset cached state
  // Reset FBO tracking; default framebuffer by default
  _currentFboId = 0;
  _ownsCurrentFbo = false;

  // Reset rendering state owned by command buffer
  _primitiveTopology = PrimitiveTopology::TriangleList;
  _boundIndexBuffer = nullptr;
  _boundVertexBuffer = nullptr;
  _shaderPipeline = nullptr;
  _boundGraphicsPipeline = nullptr;
  _boundComputePipeline = nullptr;

  _hasActivePipeline = false;
  _hasActiveVertexBuffer = false;
  _hasActiveIndexBuffer = false;
}

void GLCommandBuffer::end()
{
  validateState(CommandBufferState::Recording, "end");

  // Ensure we're not in a render pass
  if (_state == CommandBufferState::InRenderPass)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::end: Cannot end command buffer while in render pass. Call endRenderPass() first.");
  }

  _state = CommandBufferState::Executable;
}

void GLCommandBuffer::execute()
{
  validateState(CommandBufferState::Executable, "execute");

  // For OpenGL, commands are executed immediately during recording,
  // so execute() is a no-op. Other backends will submit to command queue here.
  //
  // Note: Modern engines often batch multiple command buffers together
  // for submission. Consider implementing a CommandQueue abstraction
  // that can batch submissions for better performance in Vulkan/DX12.
  // After execution, transition back to Initial state for reuse.
  _state = CommandBufferState::Initial;
}

void GLCommandBuffer::reset()
{
  // Command buffer must not be pending execution (always true for OpenGL immediate mode)
  _state = CommandBufferState::Initial;
  _usage = CommandBufferUsage::OneTimeSubmit;
  // Unbind and delete any owned FBO
  if (_ownsCurrentFbo && _currentFboId != 0)
  {
    glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    glCall(glDeleteFramebuffers(1, &_currentFboId));
  }
  _currentFboId = 0;
  _ownsCurrentFbo = false;

  // Reset rendering state owned by command buffer
  _primitiveTopology = PrimitiveTopology::TriangleList;
  _boundIndexBuffer = nullptr;
  _boundVertexBuffer = nullptr;
  _shaderPipeline = nullptr;
  _boundGraphicsPipeline = nullptr;
  _boundComputePipeline = nullptr;

  _hasActivePipeline = false;
  _hasActiveVertexBuffer = false;
  _hasActiveIndexBuffer = false;
}

// Legacy beginRenderPass(RenderTarget, ...) removed; use RenderPassBeginInfo overload

void GLCommandBuffer::beginRenderPass(const RenderPassBeginInfo &beginInfo)
{
  validateState(CommandBufferState::Recording, "beginRenderPass");

  _state = CommandBufferState::InRenderPass;

  // Bind a framebuffer: default if none provided, otherwise build a temporary FBO from desc
  if (!beginInfo.framebuffer)
  {
    glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    _currentFboId = 0;
    _ownsCurrentFbo = false;
  }
  else
  {
    const FramebufferDesc &fb = beginInfo.framebuffer->getDesc();
    GLuint fbo = 0;
    glCall(glGenFramebuffers(1, &fbo));
    glCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

    // Attach color attachments
    std::vector<GLenum> drawBuffers;
    drawBuffers.reserve(fb.colorAttachments.size());
    for (size_t i = 0; i < fb.colorAttachments.size(); ++i)
    {
      const auto &att = fb.colorAttachments[i];
      if (!att.texture)
        continue;
      auto glTex = std::static_pointer_cast<GLTexture>(att.texture);
      const TextureType ttype = glTex->getDesc().Type;
      GLenum target = getTextureTargetFromType(ttype);
      // Attach appropriately based on texture type; use layered attachment for array/cube/3D
      // Assume mip 0 for now; GL 4.1 path doesn't support views with mips/layers here
      if (ttype == TextureType::Texture2D)
      {
        glCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                                      target, glTex->getId(), 0));
      }
      else
      {
        // Layered attachments allow geometry shader to route to layers via gl_Layer
        glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), glTex->getId(), 0));
      }
      drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i));
    }
    if (!drawBuffers.empty())
    {
      glCall(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));
    }
    else
    {
      glCall(glDrawBuffer(GL_NONE));
      glCall(glReadBuffer(GL_NONE));
    }

    // Attach depth/stencil if present (best-effort: treat as depth attachment)
    if (fb.hasDepthStencilAttachment && fb.depthStencilAttachment.texture)
    {
      auto glTex = std::static_pointer_cast<GLTexture>(fb.depthStencilAttachment.texture);
      GLenum attach = GL_DEPTH_ATTACHMENT; // GL 4.1: no separate stencil ops here in our path
      const TextureType ttype = glTex->getDesc().Type;
      GLenum target = getTextureTargetFromType(ttype);
      if (ttype == TextureType::Texture2D)
      {
        glCall(glFramebufferTexture2D(GL_FRAMEBUFFER, attach, target, glTex->getId(), 0));
      }
      else
      {
        // Use layered attachment for array/cube textures so geometry shader can pick layers
        glCall(glFramebufferTexture(GL_FRAMEBUFFER, attach, glTex->getId(), 0));
      }
    }

    GLenum status = 0;
    glCall2(glCheckFramebufferStatus(GL_FRAMEBUFFER), status);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cerr << "[GL FBO] Incomplete: " << framebufferStatusToString(status)
                << " (0x" << std::hex << status << std::dec << ")" << std::endl;
      ASSERT_TRUE(false, "GLCommandBuffer::beginRenderPass: Framebuffer incomplete");
    }

    _currentFboId = fbo;
    _ownsCurrentFbo = true;
  }

  // Determine clear operations from beginInfo
  bool doClearColor = !beginInfo.clearColors.empty();
  bool doClearDepth = beginInfo.clearDepthStencil != nullptr;
  bool doClearStencil = (beginInfo.clearDepthStencil != nullptr); // same gate as depth for now

  if (doClearColor || doClearDepth || doClearStencil)
  {
    uint32 clearFlags = 0;
    if (doClearColor)
      clearFlags |= RTT_Colour;
    if (doClearDepth)
      clearFlags |= RTT_Depth;
    if (doClearStencil)
      clearFlags |= RTT_Stencil;

    // If specific color is provided, perform color clear with given value, depth/stencil defaults from struct
    if (doClearColor)
    {
      Colour c = Colour(static_cast<byte>(beginInfo.clearColors[0].r * 255.0f),
                        static_cast<byte>(beginInfo.clearColors[0].g * 255.0f),
                        static_cast<byte>(beginInfo.clearColors[0].b * 255.0f),
                        static_cast<byte>(beginInfo.clearColors[0].a * 255.0f));
      float cd = beginInfo.clearDepthStencil ? beginInfo.clearDepthStencil->depth : 1.0f;
      int cs = beginInfo.clearDepthStencil ? beginInfo.clearDepthStencil->stencil : 0;
      clearRenderTarget(clearFlags, c, cd, cs);
    }
    else
    {
      float cd = beginInfo.clearDepthStencil ? beginInfo.clearDepthStencil->depth : 1.0f;
      int cs = beginInfo.clearDepthStencil ? beginInfo.clearDepthStencil->stencil : 0;
      clearRenderTarget(clearFlags, cd, cs);
    }
  }
}

void GLCommandBuffer::nextSubpass()
{
  // Single subpass only for GL migration stage, no-op
}

void GLCommandBuffer::endRenderPass()
{
  validateState(CommandBufferState::InRenderPass, "endRenderPass");

  _state = CommandBufferState::Recording;
  // Destroy temporary FBO if we created one
  if (_ownsCurrentFbo && _currentFboId != 0)
  {
    glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    glCall(glDeleteFramebuffers(1, &_currentFboId));
  }
  _currentFboId = 0;
  _ownsCurrentFbo = false;

  // Reset pipeline and buffer state as they're render pass specific
  _hasActivePipeline = false;
  _hasActiveVertexBuffer = false;
  _hasActiveIndexBuffer = false;
}

void GLCommandBuffer::bindGraphicsPipeline(const std::shared_ptr<GraphicsPipelineState> &pipeline)
{
  validateInRenderPass("bindGraphicsPipeline");
  ASSERT_FALSE(pipeline == nullptr, "GLCommandBuffer::bindGraphicsPipeline: Pipeline cannot be null");

  _boundGraphicsPipeline = pipeline;

  // Set primitive topology and fixed function state directly
  setPrimitiveTopology(pipeline->getTopology());
  _device->setRasterizerState(pipeline->getRasterizer());
  _device->setDepthStencilState(pipeline->getDepthStencil());
  _device->setBlendState(pipeline->getBlend());
  _device->_shaderParams = pipeline->getShaderParams();

  // Bind previous resources to correct locations as in setPipelineState, only if shader params are available
  if (_device->_shaderParams)
  {
    for (uint32 i = 0; i < _device->_boundTextures.size(); i++)
    {
      if (_device->_boundTextures[i])
      {
        std::string textureName = _device->_shaderParams->getParamName(ShaderParamType::Texture, i);
        if (!textureName.empty())
        {
          auto glPs = std::static_pointer_cast<GLShader>(pipeline->getFS());
          if (glPs && glPs->hasUniform(textureName))
          {
            glPs->bindTextureUnit(textureName, i);
          }
        }
      }
    }

    for (uint32 i = 0; i < _device->_boundConstantBuffers.size(); i++)
    {
      if (_device->_boundConstantBuffers[i])
      {
        auto uniformBufferName = _device->_shaderParams->getParamName(ShaderParamType::ConstBuffer, i);
        auto glVs = std::static_pointer_cast<GLShader>(pipeline->getVS());
        if (glVs && glVs->hasUniform(uniformBufferName))
        {
          glVs->bindUniformBlock(uniformBufferName, i);
        }
        auto glPs = std::static_pointer_cast<GLShader>(pipeline->getFS());
        if (glPs && glPs->hasUniform(uniformBufferName))
        {
          glPs->bindUniformBlock(uniformBufferName, i);
        }
        auto glGs = std::static_pointer_cast<GLShader>(pipeline->getGS());
        if (glGs && glGs->hasUniform(uniformBufferName))
        {
          glGs->bindUniformBlock(uniformBufferName, i);
        }
        auto glHs = std::static_pointer_cast<GLShader>(pipeline->getHS());
        if (glHs && glHs->hasUniform(uniformBufferName))
        {
          glHs->bindUniformBlock(uniformBufferName, i);
        }
        auto glDs = std::static_pointer_cast<GLShader>(pipeline->getDS());
        if (glDs && glDs->hasUniform(uniformBufferName))
        {
          glDs->bindUniformBlock(uniformBufferName, i);
        }
      }
    }
  }

  _device->_shaderStateChanged = true;
  _hasActivePipeline = true;
}

void GLCommandBuffer::bindComputePipeline(const std::shared_ptr<ComputePipelineState> &pipeline)
{
  // GL 4.1 has no compute support; store and warn once
  _boundComputePipeline = pipeline;
  if (!_warnedComputeUnsupported)
  {
    _warnedComputeUnsupported = true;
    std::cerr << "[GL 4.1] Compute not supported; ignoring bindComputePipeline." << std::endl;
  }
}

void GLCommandBuffer::bindResourceSet(const std::unique_ptr<IResourceSet> &resourceSet, uint32 setIndex)
{
  validateInRenderPass("bindResourceSet");
  ASSERT_FALSE(resourceSet == nullptr, "GLCommandBuffer::bindResourceSet: Resource set cannot be null");

  // If pipeline or shader params are not ready, bind the raw GL resources and treat as a no-op for shader interface mapping
  if (_boundGraphicsPipeline == nullptr || _device == nullptr || _device->_shaderParams == nullptr)
  {
    resourceSet->bind(_device, setIndex);
    return;
  }

  std::vector<const IResourceSet *> sets;
  sets.push_back(resourceSet.get());
  // PipelineLayout not strictly needed on GL; pass nullptr
  bindDescriptorSets(PipelineBindPoint::Graphics, nullptr, setIndex, sets);
}

void GLCommandBuffer::bindDescriptorSets(PipelineBindPoint bindPoint,
                                         const std::shared_ptr<PipelineLayout> &layout,
                                         uint32 firstSet,
                                         const std::vector<const IResourceSet *> &sets,
                                         const uint32 *dynamicOffsets,
                                         uint32 dynamicOffsetCount)
{
  (void)bindPoint;
  (void)layout;
  (void)dynamicOffsets;
  (void)dynamicOffsetCount;
  validateInRenderPass("bindDescriptorSets");

  // Early-out: nothing to bind
  if (sets.empty())
  {
    return;
  }

  // If no graphics pipeline is bound or shader params are unavailable, there is nothing meaningful
  // we can do in the GL path. Treat as a safe no-op to match migration expectations.
  if (_boundGraphicsPipeline == nullptr || _device == nullptr || _device->_shaderParams == nullptr)
  {
    return;
  }

  // Best-effort GL mapping: iterate each set and call its bind with set index sequencing
  for (uint32 i = 0; i < sets.size(); ++i)
  {
    const IResourceSet *set = sets[i];
    if (set)
    {
      // We only have const IResourceSet*; bind is const and takes shared_ptr<RenderDevice>
      set->bind(_device, firstSet + i);

      // After binding GL resources to binding points, hook up shader interface (uniform blocks and samplers)
      if (_boundGraphicsPipeline && _device && _device->_shaderParams)
      {
        auto glResourceSet = dynamic_cast<const GLResourceSet *>(set);
        if (glResourceSet)
        {
          // Apply dynamic offsets (best-effort): sequentially map to UNIFORM_BUFFER bindings
          uint32 dynIdx = 0;
          for (const auto &binding : glResourceSet->getBindings())
          {
            if (binding.type == ResourceType::UNIFORM_BUFFER)
            {
              if (dynamicOffsets && dynIdx < dynamicOffsetCount && !binding.resources.empty())
              {
                const GpuBuffer *ub = static_cast<const GpuBuffer *>(binding.resources[0]);
                if (ub)
                {
                  GLuint handle = static_cast<GLuint>(reinterpret_cast<uintptr_t>(ub->getNativeHandle()));
                  uint32 offsetBytes = dynamicOffsets[dynIdx++];
                  uint64 sizeBytes = ub->getSizeBytes();
                  if (offsetBytes < sizeBytes)
                  {
                    glCall(glBindBufferRange(GL_UNIFORM_BUFFER, binding.binding, handle, offsetBytes, static_cast<GLsizeiptr>(sizeBytes - offsetBytes)));
                  }
                }
              }
              auto uniformBufferName = _device->_shaderParams->getParamName(ShaderParamType::ConstBuffer, binding.binding);
              if (!uniformBufferName.empty())
              {
                auto glVs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getVS());
                if (glVs && glVs->hasUniform(uniformBufferName))
                  glVs->bindUniformBlock(uniformBufferName, binding.binding);
                auto glPs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getFS());
                if (glPs && glPs->hasUniform(uniformBufferName))
                  glPs->bindUniformBlock(uniformBufferName, binding.binding);
                auto glGs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getGS());
                if (glGs && glGs->hasUniform(uniformBufferName))
                  glGs->bindUniformBlock(uniformBufferName, binding.binding);
                auto glHs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getHS());
                if (glHs && glHs->hasUniform(uniformBufferName))
                  glHs->bindUniformBlock(uniformBufferName, binding.binding);
                auto glDs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getDS());
                if (glDs && glDs->hasUniform(uniformBufferName))
                  glDs->bindUniformBlock(uniformBufferName, binding.binding);
              }
            }
            else if (binding.type == ResourceType::TEXTURE_2D || binding.type == ResourceType::TEXTURE_CUBE)
            {
              auto textureName = _device->_shaderParams->getParamName(ShaderParamType::Texture, binding.binding);
              if (!textureName.empty())
              {
                auto glVs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getVS());
                if (glVs && glVs->hasUniform(textureName))
                  glVs->bindTextureUnit(textureName, binding.binding);
                auto glPs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getFS());
                if (glPs && glPs->hasUniform(textureName))
                  glPs->bindTextureUnit(textureName, binding.binding);
                auto glGs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getGS());
                if (glGs && glGs->hasUniform(textureName))
                  glGs->bindTextureUnit(textureName, binding.binding);
                auto glHs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getHS());
                if (glHs && glHs->hasUniform(textureName))
                  glHs->bindTextureUnit(textureName, binding.binding);
                auto glDs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getDS());
                if (glDs && glDs->hasUniform(textureName))
                  glDs->bindTextureUnit(textureName, binding.binding);
              }
            }
          }
        }
      }
    }
  }
}

void GLCommandBuffer::pushConstants(const std::shared_ptr<PipelineLayout> &layout,
                                    uint32 stageMask,
                                    uint32 offset,
                                    uint32 size,
                                    const void *data)
{
  (void)layout;
  (void)stageMask;
  validateInRenderPass("pushConstants");

  // GL has no push constants; emulate via a small transient UBO bound at a reserved binding point.
  // Use central limits and reserved binding definitions.
  constexpr GLuint kPushBinding = static_cast<GLuint>(RenderLimits::GLPushConstantsBinding);
  static GLuint sPushUbo = 0;
  if (sPushUbo == 0)
  {
    glCall(glGenBuffers(1, &sPushUbo));
    glCall(glBindBuffer(GL_UNIFORM_BUFFER, sPushUbo));
    glCall(glBufferData(GL_UNIFORM_BUFFER, RenderLimits::PushConstantsMaxBytes, nullptr, GL_DYNAMIC_DRAW));
  }
  glCall(glBindBuffer(GL_UNIFORM_BUFFER, sPushUbo));
  // Ensure buffer large enough for offset+size (cap to 256 for now)
  if (offset + size > RenderLimits::PushConstantsMaxBytes)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::pushConstants: push constant size exceeds 256 bytes cap");
    return;
  }
  glCall(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
  glCall(glBindBufferBase(GL_UNIFORM_BUFFER, kPushBinding, sPushUbo));

  // Ensure current shader program(s) have the expected uniform block bound to the reserved binding.
  // We standardize on the block name "PushConstants" for GL path.
  if (_boundGraphicsPipeline)
  {
    const char *kBlockName = "PushConstants";
    auto glVs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getVS());
    if (glVs && glVs->hasUniform(kBlockName))
    {
      glVs->bindUniformBlock(kBlockName, kPushBinding);
    }
    auto glPs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getFS());
    if (glPs && glPs->hasUniform(kBlockName))
    {
      glPs->bindUniformBlock(kBlockName, kPushBinding);
    }
    auto glGs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getGS());
    if (glGs && glGs->hasUniform(kBlockName))
    {
      glGs->bindUniformBlock(kBlockName, kPushBinding);
    }
    auto glHs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getHS());
    if (glHs && glHs->hasUniform(kBlockName))
    {
      glHs->bindUniformBlock(kBlockName, kPushBinding);
    }
    auto glDs = std::static_pointer_cast<GLShader>(_boundGraphicsPipeline->getDS());
    if (glDs && glDs->hasUniform(kBlockName))
    {
      glDs->bindUniformBlock(kBlockName, kPushBinding);
    }
  }

  // Note: Shaders must declare a uniform block bound to binding 15 for push constants in GL path.
}

void GLCommandBuffer::bindVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer,
                                       uint32 bindingIndex,
                                       uint64 offset)
{
  validateInRenderPass("bindVertexBuffer");

  ASSERT_FALSE(vertexBuffer == nullptr, "GLCommandBuffer::bindVertexBuffer: Vertex buffer cannot be null");

  // OpenGL doesn't support multiple vertex buffer bindings or offsets in the same way as modern APIs
  if (bindingIndex != 0)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::bindVertexBuffer: OpenGL implementation only supports binding index 0");
  }

  if (offset != 0)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::bindVertexBuffer: OpenGL implementation doesn't support vertex buffer offsets");
  }

  // Directly handle vertex buffer binding in command buffer
  auto glVertexBuffer = std::static_pointer_cast<GLVertexBuffer>(vertexBuffer);
  _boundVertexBuffer = glVertexBuffer;
  _hasActiveVertexBuffer = true;
}

void GLCommandBuffer::bindIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer, uint64 offset)
{
  validateInRenderPass("bindIndexBuffer");

  ASSERT_FALSE(indexBuffer == nullptr, "GLCommandBuffer::bindIndexBuffer: Index buffer cannot be null");

  if (offset != 0)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::bindIndexBuffer: OpenGL implementation doesn't support index buffer offsets");
  }

  // Directly handle index buffer binding in command buffer
  auto glIndexBuffer = std::static_pointer_cast<GLIndexBuffer>(indexBuffer);
  _boundIndexBuffer = glIndexBuffer;
  _hasActiveIndexBuffer = true;
}

void GLCommandBuffer::setViewport(const ViewportDesc &viewport)
{
  validateInRenderPass("setViewport");

  // Check if viewport has changed to avoid unnecessary OpenGL calls
  if (_device->_viewportDesc.TopLeftX != viewport.TopLeftX ||
      _device->_viewportDesc.TopLeftY != viewport.TopLeftY ||
      _device->_viewportDesc.Width != viewport.Width ||
      _device->_viewportDesc.Height != viewport.Height)
  {
    glCall(glViewport(viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height));
    _device->_viewportDesc = viewport;
  }
}

void GLCommandBuffer::setScissor(const ScissorDesc &scissor)
{
  validateInRenderPass("setScissor");

  // Validate scissor dimensions against render target bounds
  ASSERT_FALSE(scissor.X < 0.0f || scissor.X > _device->_desc.RenderWidth, "Scissor X-Pos exceeds render dimensions");
  ASSERT_FALSE(scissor.Y < 0.0f || scissor.Y > _device->_desc.RenderHeight, "Scissor Y-Pos exceeds render dimensions");
  ASSERT_FALSE(scissor.W < 0.0f || scissor.W > _device->_desc.RenderWidth, "Scissor width exceeds render dimensions");
  ASSERT_FALSE(scissor.H < 0.0f || scissor.H > _device->_desc.RenderHeight, "Scissor height exceeds render dimensions");

  // Check if scissor dimensions have changed to avoid unnecessary OpenGL calls
  if (scissor.X != _device->_scissorDesc.X || scissor.Y != _device->_scissorDesc.Y ||
      scissor.W != _device->_scissorDesc.W || scissor.H != _device->_scissorDesc.H)
  {
    glCall(glScissor(scissor.X, scissor.Y, scissor.W, scissor.H));
    _device->_scissorDesc = scissor;
  }
}

void GLCommandBuffer::setPrimitiveTopology(PrimitiveTopology topology)
{
  // Command buffer must be in render pass
  validateState(CommandBufferState::InRenderPass, "setPrimitiveTopology");

  _primitiveTopology = topology;
}

void GLCommandBuffer::draw(uint32 vertexCount,
                           uint32 instanceCount,
                           uint32 firstVertex,
                           uint32 firstInstance)
{
  validateInRenderPass("draw");

  ASSERT_TRUE(_hasActivePipeline, "GLCommandBuffer::draw: No pipeline state bound");

  if (instanceCount != 1)
  {
    ASSERT_TRUE(instanceCount > 1, "GLCommandBuffer::draw: OpenGL implementation doesn't support instanced rendering in this path");
  }

  if (firstInstance != 0)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::draw: OpenGL implementation doesn't support firstInstance parameter");
  }

  // Execute the draw call directly in command buffer
  beginDraw();
  if (instanceCount > 1)
  {
    glCall(glDrawArraysInstanced(getPrimitiveTopology(_primitiveTopology), firstVertex, vertexCount, instanceCount));
  }
  else
  {
    glCall(glDrawArrays(getPrimitiveTopology(_primitiveTopology), firstVertex, vertexCount));
  }
  endDraw();
}

void GLCommandBuffer::drawIndexed(uint32 indexCount,
                                  uint32 instanceCount,
                                  uint32 firstIndex,
                                  int32 vertexOffset,
                                  uint32 firstInstance)
{
  validateInRenderPass("drawIndexed");

  ASSERT_TRUE(_hasActivePipeline, "GLCommandBuffer::drawIndexed: No pipeline state bound");
  ASSERT_TRUE(_hasActiveVertexBuffer, "GLCommandBuffer::drawIndexed: No vertex buffer bound");
  ASSERT_TRUE(_hasActiveIndexBuffer, "GLCommandBuffer::drawIndexed: No index buffer bound");

  if (instanceCount != 1)
  {
    ASSERT_TRUE(instanceCount > 1, "GLCommandBuffer::drawIndexed: OpenGL implementation doesn't support instanced rendering in this path");
  }

  if (firstInstance != 0)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::drawIndexed: OpenGL implementation doesn't support firstInstance parameter");
  }

  // Execute the indexed draw call directly in command buffer
  beginDraw();
  ASSERT_FALSE(_boundIndexBuffer == nullptr, "No index buffer has been bound");
  glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _boundIndexBuffer->getId()));

  GLenum idxType = _boundIndexBuffer->getIndexType() == IndexType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
  uint32 idxTypeByteCount = IndexBuffer::getBytesPerIndex(_boundIndexBuffer->getIndexType());
  if (instanceCount > 1)
  {
    glCall(glDrawElementsInstancedBaseVertex(getPrimitiveTopology(_primitiveTopology), indexCount, idxType, reinterpret_cast<GLvoid *>(idxTypeByteCount * firstIndex), instanceCount, vertexOffset));
  }
  else
  {
    glCall(glDrawElementsBaseVertex(getPrimitiveTopology(_primitiveTopology), indexCount, idxType, reinterpret_cast<GLvoid *>(idxTypeByteCount * firstIndex), vertexOffset));
  }
  endDraw();
}

void GLCommandBuffer::drawIndirect(const std::shared_ptr<GpuBuffer> &argsBuffer, uint64 offset)
{
  validateInRenderPass("drawIndirect");
  ASSERT_TRUE(_hasActivePipeline, "GLCommandBuffer::drawIndirect: No pipeline state bound");
  auto glBuf = std::static_pointer_cast<GLGpuBuffer>(argsBuffer);
  ASSERT_FALSE(glBuf == nullptr, "drawIndirect: argsBuffer must be GLGpuBuffer");

  beginDraw();
  glCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, glBuf->GetId()));
  // glDrawArraysIndirect introduced in GL 4.0 (ARB_draw_indirect)
  glCall(glDrawArraysIndirect(getPrimitiveTopology(_primitiveTopology), reinterpret_cast<const void *>(offset)));
  glCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
  endDraw();
}

void GLCommandBuffer::drawIndexedIndirect(const std::shared_ptr<GpuBuffer> &argsBuffer, uint64 offset)
{
  validateInRenderPass("drawIndexedIndirect");
  ASSERT_TRUE(_hasActivePipeline, "GLCommandBuffer::drawIndexedIndirect: No pipeline state bound");
  ASSERT_TRUE(_hasActiveIndexBuffer, "GLCommandBuffer::drawIndexedIndirect: No index buffer bound");

  auto glBuf = std::static_pointer_cast<GLGpuBuffer>(argsBuffer);
  ASSERT_FALSE(glBuf == nullptr, "drawIndexedIndirect: argsBuffer must be GLGpuBuffer");

  beginDraw();
  glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _boundIndexBuffer->getId()));
  glCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, glBuf->GetId()));
  GLenum idxType = _boundIndexBuffer->getIndexType() == IndexType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
  glCall(glDrawElementsIndirect(getPrimitiveTopology(_primitiveTopology), idxType, reinterpret_cast<const void *>(offset)));
  glCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
  endDraw();
}

void GLCommandBuffer::dispatch(uint32 x, uint32 y, uint32 z)
{
  // No-op on GL 4.1; warn once
  (void)x;
  (void)y;
  (void)z;
  if (!_warnedComputeUnsupported)
  {
    _warnedComputeUnsupported = true;
    std::cerr << "[GL 4.1] Compute not supported; ignoring dispatch." << std::endl;
  }
}

void GLCommandBuffer::dispatchIndirect(const std::shared_ptr<GpuBuffer> &buffer, uint64 offset)
{
  // No-op on GL 4.1; warn once
  (void)buffer;
  (void)offset;
  if (!_warnedComputeUnsupported)
  {
    _warnedComputeUnsupported = true;
    std::cerr << "[GL 4.1] Compute not supported; ignoring dispatchIndirect." << std::endl;
  }
}

void GLCommandBuffer::copyBuffer(const std::shared_ptr<GpuBuffer> &srcBuffer,
                                 const std::shared_ptr<GpuBuffer> &dstBuffer,
                                 uint64 srcOffset,
                                 uint64 dstOffset,
                                 uint64 size)
{
  validateOutsideRenderPass("copyBuffer");

  ASSERT_FALSE(srcBuffer == nullptr, "GLCommandBuffer::copyBuffer: Source buffer cannot be null");
  ASSERT_FALSE(dstBuffer == nullptr, "GLCommandBuffer::copyBuffer: Destination buffer cannot be null");
  // Use GL copy buffers path if both are GL buffers
  auto glSrc = std::static_pointer_cast<GLGpuBuffer>(srcBuffer);
  auto glDst = std::static_pointer_cast<GLGpuBuffer>(dstBuffer);
  ASSERT_FALSE(glSrc == nullptr || glDst == nullptr, "GLCommandBuffer::copyBuffer: Buffers must be GLGpuBuffer");
  glCall(glBindBuffer(GL_COPY_READ_BUFFER, glSrc->GetId()));
  glCall(glBindBuffer(GL_COPY_WRITE_BUFFER, glDst->GetId()));
  glCall(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, size));
  glCall(glBindBuffer(GL_COPY_READ_BUFFER, 0));
  glCall(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
}

void GLCommandBuffer::copyBufferToTexture(const std::shared_ptr<GpuBuffer> &srcBuffer,
                                          const std::shared_ptr<Texture> &dstTexture,
                                          uint64 bufferOffset,
                                          uint32 mipLevel,
                                          uint32 arrayLayer)
{
  validateOutsideRenderPass("copyBufferToTexture");

  ASSERT_FALSE(srcBuffer == nullptr, "GLCommandBuffer::copyBufferToTexture: Source buffer cannot be null");
  ASSERT_FALSE(dstTexture == nullptr, "GLCommandBuffer::copyBufferToTexture: Destination texture cannot be null");
  auto glSrc = std::static_pointer_cast<GLGpuBuffer>(srcBuffer);
  auto glDstTex = std::static_pointer_cast<GLTexture>(dstTexture);
  ASSERT_FALSE(glSrc == nullptr || glDstTex == nullptr, "GLCommandBuffer::copyBufferToTexture: Types must be GLGpuBuffer/GLTexture");

  // Determine texture format to know pixel store parameters
  auto mapTexFmt = [](TextureFormat tf, GLenum &format, GLenum &type)
  {
    switch (tf)
    {
    case TextureFormat::R8:
      format = GL_RED;
      type = GL_UNSIGNED_BYTE;
      return;
    case TextureFormat::RG8:
      format = GL_RG;
      type = GL_UNSIGNED_BYTE;
      return;
    case TextureFormat::RGB8:
      format = GL_RGB;
      type = GL_UNSIGNED_BYTE;
      return;
    case TextureFormat::RGBA8:
      format = GL_RGBA;
      type = GL_UNSIGNED_BYTE;
      return;
    case TextureFormat::RGB16F:
      format = GL_RGB;
      type = GL_FLOAT;
      return;
    case TextureFormat::RGB32F:
      format = GL_RGB;
      type = GL_FLOAT;
      return;
    case TextureFormat::RGBA16F:
      format = GL_RGBA;
      type = GL_FLOAT;
      return;
    case TextureFormat::D32:
    case TextureFormat::D32F:
    case TextureFormat::D24:
    case TextureFormat::D24S8:
    default:
      // Depth/stencil copies via PBO not supported in this helper
      format = GL_RGBA;
      type = GL_UNSIGNED_BYTE;
      return;
    }
  };
  GLenum format = GL_RGBA, type = GL_UNSIGNED_BYTE;
  mapTexFmt(glDstTex->getDesc().Format, format, type);
  GLenum target = getTextureTargetFromType(glDstTex->getDesc().Type);

  // Bind PBO to unpack data from buffer
  glCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glSrc->GetId()));
  glCall(glBindTexture(target, glDstTex->getId()));

  // For now, support Texture2D single layer
  if (glDstTex->getDesc().Type == TextureType::Texture2D)
  {
    const uint32 w = glDstTex->getDesc().Width >> mipLevel;
    const uint32 h = glDstTex->getDesc().Height >> mipLevel;
    glCall(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    glCall(glTexSubImage2D(target, mipLevel, 0, 0, w, h, format, type, reinterpret_cast<const GLvoid *>(bufferOffset)));
  }
  else
  {
    ASSERT_TRUE(false, "GLCommandBuffer::copyBufferToTexture: Only Texture2D supported in this path");
  }

  glCall(glBindTexture(target, 0));
  glCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
}

void GLCommandBuffer::updateBuffer(const std::shared_ptr<GpuBuffer> &buffer,
                                   uint64 offset,
                                   uint64 size,
                                   const void *data,
                                   AccessType accessType)
{
  validateOutsideRenderPass("updateBuffer");

  ASSERT_FALSE(buffer == nullptr, "GLCommandBuffer::updateBuffer: Buffer cannot be null");
  ASSERT_FALSE(data == nullptr, "GLCommandBuffer::updateBuffer: Data cannot be null");
  ASSERT_TRUE(size > 0, "GLCommandBuffer::updateBuffer: Size must be greater than 0");
  auto glBuf = std::static_pointer_cast<GLGpuBuffer>(buffer);
  ASSERT_FALSE(glBuf == nullptr, "GLCommandBuffer::updateBuffer: Buffer must be GLGpuBuffer");
  auto bufTarget = [](BufferType bt)
  {
    switch (bt)
    {
    case BufferType::Vertex:
      return GL_ARRAY_BUFFER;
    case BufferType::Index:
      return GL_ELEMENT_ARRAY_BUFFER;
    case BufferType::Constant:
      return GL_UNIFORM_BUFFER;
    default:
      return GL_ARRAY_BUFFER;
    }
  }(glBuf->getDesc().BufferType);
  glCall(glBindBuffer(bufTarget, glBuf->GetId()));
  glCall(glBufferSubData(bufTarget, offset, size, data));
  glCall(glBindBuffer(bufTarget, 0));
}

void GLCommandBuffer::copyTexture(const std::shared_ptr<Texture> &src,
                                  const std::shared_ptr<Texture> &dst,
                                  uint32 srcMipLevel,
                                  uint32 dstMipLevel)
{
  validateOutsideRenderPass("copyTexture");
  ASSERT_FALSE(src == nullptr || dst == nullptr, "copyTexture: null textures");
  auto glSrc = std::static_pointer_cast<GLTexture>(src);
  auto glDst = std::static_pointer_cast<GLTexture>(dst);
  ASSERT_FALSE(glSrc == nullptr || glDst == nullptr, "copyTexture: must be GLTexture");
  ASSERT_TRUE(glSrc->getDesc().Type == TextureType::Texture2D && glDst->getDesc().Type == TextureType::Texture2D, "copyTexture: only Texture2D supported");

  // Use FBO blit between textures by attaching as read/draw
  GLuint fboRead = 0, fboDraw = 0;
  glCall(glGenFramebuffers(1, &fboRead));
  glCall(glGenFramebuffers(1, &fboDraw));
  glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRead));
  glCall(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glSrc->getId(), srcMipLevel));
  glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDraw));
  glCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glDst->getId(), dstMipLevel));
  GLsizei sw = static_cast<GLsizei>(glSrc->getDesc().Width >> srcMipLevel);
  GLsizei sh = static_cast<GLsizei>(glSrc->getDesc().Height >> srcMipLevel);
  GLsizei dw = static_cast<GLsizei>(glDst->getDesc().Width >> dstMipLevel);
  GLsizei dh = static_cast<GLsizei>(glDst->getDesc().Height >> dstMipLevel);
  ASSERT_TRUE(sw == dw && sh == dh, "copyTexture: mip sizes must match; use blitTexture for scaling");
  glCall(glBlitFramebuffer(0, 0, sw, sh, 0, 0, dw, dh, GL_COLOR_BUFFER_BIT, GL_NEAREST));
  glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
  glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
  glCall(glDeleteFramebuffers(1, &fboRead));
  glCall(glDeleteFramebuffers(1, &fboDraw));
}

void GLCommandBuffer::blitTexture(const std::shared_ptr<Texture> &src,
                                  const std::shared_ptr<Texture> &dst,
                                  bool linearFilter,
                                  uint32 srcMipLevel,
                                  uint32 dstMipLevel)
{
  validateOutsideRenderPass("blitTexture");
  ASSERT_FALSE(src == nullptr || dst == nullptr, "blitTexture: null textures");
  auto glSrc = std::static_pointer_cast<GLTexture>(src);
  auto glDst = std::static_pointer_cast<GLTexture>(dst);
  ASSERT_FALSE(glSrc == nullptr || glDst == nullptr, "blitTexture: must be GLTexture");
  ASSERT_TRUE(glSrc->getDesc().Type == TextureType::Texture2D && glDst->getDesc().Type == TextureType::Texture2D, "blitTexture: only Texture2D supported");

  GLuint fboRead = 0, fboDraw = 0;
  glCall(glGenFramebuffers(1, &fboRead));
  glCall(glGenFramebuffers(1, &fboDraw));
  glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRead));
  glCall(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glSrc->getId(), srcMipLevel));
  glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDraw));
  glCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glDst->getId(), dstMipLevel));
  GLsizei sw = static_cast<GLsizei>(glSrc->getDesc().Width >> srcMipLevel);
  GLsizei sh = static_cast<GLsizei>(glSrc->getDesc().Height >> srcMipLevel);
  GLsizei dw = static_cast<GLsizei>(glDst->getDesc().Width >> dstMipLevel);
  GLsizei dh = static_cast<GLsizei>(glDst->getDesc().Height >> dstMipLevel);
  glCall(glBlitFramebuffer(0, 0, sw, sh, 0, 0, dw, dh, GL_COLOR_BUFFER_BIT, linearFilter ? GL_LINEAR : GL_NEAREST));
  glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
  glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
  glCall(glDeleteFramebuffers(1, &fboRead));
  glCall(glDeleteFramebuffers(1, &fboDraw));
}

void GLCommandBuffer::resolveTexture(const std::shared_ptr<Texture> &srcMsaa,
                                     const std::shared_ptr<Texture> &dstSingle,
                                     uint32 srcMipLevel,
                                     uint32 dstMipLevel)
{
  // Same as blit, but conceptually for MSAA->single; rely on GL to resolve when src is multisample renderbuffer/texture
  blitTexture(srcMsaa, dstSingle, false, srcMipLevel, dstMipLevel);
}

void GLCommandBuffer::generateMips(const std::shared_ptr<Texture> &texture)
{
  validateOutsideRenderPass("generateMips");
  ASSERT_FALSE(texture == nullptr, "generateMips: texture null");
  auto glTex = std::static_pointer_cast<GLTexture>(texture);
  ASSERT_FALSE(glTex == nullptr, "generateMips: GLTexture expected");
  glTex->generateMips();
}

void GLCommandBuffer::blitDepthToDefault(const std::shared_ptr<Texture> &srcDepth)
{
  validateOutsideRenderPass("blitDepthToDefault");
  ASSERT_FALSE(srcDepth == nullptr, "blitDepthToDefault: srcDepth null");
  auto glSrc = std::static_pointer_cast<GLTexture>(srcDepth);
  ASSERT_FALSE(glSrc == nullptr, "blitDepthToDefault: GLTexture expected");
  // Only Texture2D depth supported for this helper
  ASSERT_TRUE(glSrc->getDesc().Type == TextureType::Texture2D || glSrc->getDesc().Type == TextureType::Texture2DArray,
              "blitDepthToDefault: only 2D or 2DArray depth supported");

  // Create a read FBO attaching the depth texture, draw FBO is the default (0)
  GLuint fboRead = 0;
  glCall(glGenFramebuffers(1, &fboRead));
  glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRead));

  const TextureType ttype = glSrc->getDesc().Type;
  if (ttype == TextureType::Texture2D)
  {
    GLenum target = getTextureTargetFromType(ttype);
    glCall(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, glSrc->getId(), 0));
  }
  else
  {
    // For array/cube textures, attach a single layer (0). Geometry will have rendered to layers.
    // Using glFramebufferTextureLayer is valid in GL 3.0+.
    glCall(glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glSrc->getId(), 0, 0));
  }

  // Bind default framebuffer as draw target
  glCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));

  GLsizei w = static_cast<GLsizei>(glSrc->getDesc().Width);
  GLsizei h = static_cast<GLsizei>(glSrc->getDesc().Height);
  glCall(glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST));

  glCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
  glCall(glDeleteFramebuffers(1, &fboRead));
}

void GLCommandBuffer::memoryBarrier(uint32 srcStage, uint32 dstStage)
{
  (void)srcStage;
  (void)dstStage;
  // Best-effort: if glMemoryBarrier is available, issue a full barrier to order prior writes
  if (glMemoryBarrierAvailable())
  {
#if defined(GL_VERSION_4_2) && defined(GL_ALL_BARRIER_BITS)
    glCall(glMemoryBarrier(GL_ALL_BARRIER_BITS));
#endif
  }
}

void GLCommandBuffer::pipelineBarrier(uint32 srcStages,
                                      uint32 dstStages,
                                      uint32 memoryDeps,
                                      const std::vector<BufferBarrier> &bufferBarriers,
                                      const std::vector<ImageBarrier> &imageBarriers)
{
  (void)srcStages;
  (void)dstStages;
  validateOutsideRenderPass("pipelineBarrier");
  (void)bufferBarriers;
  (void)imageBarriers;

  // Best-effort GL mapping: use glMemoryBarrier when available
  if (glMemoryBarrierAvailable())
  {
    GLbitfield glBits = mapMemoryDepsToGL(memoryDeps);
#if defined(GL_VERSION_4_2) && defined(GL_ALL_BARRIER_BITS)
    if (glBits == 0 && memoryDeps != 0)
      glBits = GL_ALL_BARRIER_BITS;
#endif
    if (glBits != 0)
    {
#if defined(GL_VERSION_4_2)
      glCall(glMemoryBarrier(glBits));
#endif
    }
  }
}

void GLCommandBuffer::transition(const std::shared_ptr<ImageView> &view,
                                 ResourceState oldState,
                                 ResourceState newState,
                                 ImageAspect aspect)
{
  (void)oldState;
  (void)newState;
  (void)aspect;
  // On GL 4.1 transitions are implicit; we still insert a conservative barrier to aid
  // correctness around copies and subsequent sampling.
  const uint32 deps = MemoryDeps::ShaderRead | MemoryDeps::ColorAttachmentWrite | MemoryDeps::DepthStencilWrite | MemoryDeps::TransferWrite;
  pipelineBarrier(PipelineStage::AllCommands, PipelineStage::AllCommands, deps, {}, {});
}

void GLCommandBuffer::writeTimestamp(const std::shared_ptr<IQueryPool> &pool, uint32 index)
{
  (void)index;
  validateOutsideRenderPass("writeTimestamp");
  auto glPool = std::dynamic_pointer_cast<GLQueryPool>(pool);
  ASSERT_FALSE(glPool == nullptr, "writeTimestamp: GLQueryPool required");
  // Use GL_TIMESTAMP into a query object
  glCall(glQueryCounter(glPool->getId(index), GL_TIMESTAMP));
}

void GLCommandBuffer::beginQuery(const std::shared_ptr<IQueryPool> &pool, uint32 index)
{
  auto glPool = std::dynamic_pointer_cast<GLQueryPool>(pool);
  ASSERT_FALSE(glPool == nullptr, "beginQuery: GLQueryPool required");
  GLenum target = GL_TIME_ELAPSED;
  switch (glPool->getType())
  {
  case QueryType::TimeElapsed:
    target = GL_TIME_ELAPSED;
    break;
  case QueryType::SamplesPassed:
    target = GL_SAMPLES_PASSED;
    break;
  case QueryType::Timestamp:
    target = GL_TIMESTAMP;
    break; // will be used with writeTimestamp
  }
  if (target == GL_TIMESTAMP)
  {
    // For timestamp, just issue a counter; begin/end are not used
    glCall(glQueryCounter(glPool->getId(index), GL_TIMESTAMP));
  }
  else
  {
    glCall(glBeginQuery(target, glPool->getId(index)));
  }
}

void GLCommandBuffer::endQuery(const std::shared_ptr<IQueryPool> &pool, uint32 index)
{
  auto glPool = std::dynamic_pointer_cast<GLQueryPool>(pool);
  ASSERT_FALSE(glPool == nullptr, "endQuery: GLQueryPool required");
  GLenum target = GL_TIME_ELAPSED;
  switch (glPool->getType())
  {
  case QueryType::TimeElapsed:
    target = GL_TIME_ELAPSED;
    break;
  case QueryType::SamplesPassed:
    target = GL_SAMPLES_PASSED;
    break;
  case QueryType::Timestamp:
    target = GL_TIMESTAMP;
    break;
  }
  if (target == GL_TIMESTAMP)
  {
    // Nothing to end for timestamp
    return;
  }
  glCall(glEndQuery(target));
}

void GLCommandBuffer::resolveQueryData(const std::shared_ptr<IQueryPool> &pool, uint32 first, uint32 count, uint64 *dst)
{
  ASSERT_FALSE(dst == nullptr, "resolveQueryData: dst is null");
  auto glPool = std::dynamic_pointer_cast<GLQueryPool>(pool);
  ASSERT_FALSE(glPool == nullptr, "resolveQueryData: GLQueryPool required");
  for (uint32 i = 0; i < count; ++i)
  {
    GLuint id = glPool->getId(first + i);
    GLuint64 value = 0;
    glCall(glGetQueryObjectui64v(id, GL_QUERY_RESULT, &value));
    dst[i] = static_cast<uint64>(value);
  }
}

// setRenderTarget removed with deprecated RenderTarget adapter

void GLCommandBuffer::clearRenderTarget(uint32 buffers, const Colour &colour, float32 depth, int32 stencil)
{
  validateInRenderPass("clearRenderTarget");

  // Save current scissor dimensions and ensure full-surface clear without relying on pipeline state
  auto currentScissorDimensions = _device->getScissorDimensions();
  bool restoreScissor = false;
  if (_boundGraphicsPipeline && _boundGraphicsPipeline->getRasterizer() && _boundGraphicsPipeline->getRasterizer()->isScissorEnabled())
  {
    restoreScissor = true;
  }
  // Disable scissor to guarantee full clear
  glCall(glDisable(GL_SCISSOR_TEST));

  GLbitfield flags = 0;
  if (buffers & RTT_Colour)
  {
    flags |= GL_COLOR_BUFFER_BIT;
    glCall(glClearColor(colour[0], colour[1], colour[2], colour[3]));
  }
  if (buffers & RTT_Depth)
  {
    flags |= GL_DEPTH_BUFFER_BIT;
    glCall(glClearDepth(depth));
  }
  if (buffers & RTT_Stencil)
  {
    flags |= GL_STENCIL_BUFFER_BIT;
    glCall(glClearStencil(stencil));
  }
  glCall(glClear(flags));

  // Restore scissor state if it was expected to be enabled by pipeline
  if (restoreScissor)
  {
    glCall(glEnable(GL_SCISSOR_TEST));
    glCall(glScissor(currentScissorDimensions.X, currentScissorDimensions.Y, currentScissorDimensions.W, currentScissorDimensions.H));
  }
}

void GLCommandBuffer::clearRenderTarget(uint32 buffers, float32 depth, int32 stencil)
{
  clearRenderTarget(buffers, Colour::Black, depth, stencil);
}

CommandBufferState GLCommandBuffer::getState() const
{
  return _state;
}

bool GLCommandBuffer::isRecording() const
{
  return _state == CommandBufferState::Recording || _state == CommandBufferState::InRenderPass;
}

bool GLCommandBuffer::isInRenderPass() const
{
  return _state == CommandBufferState::InRenderPass;
}

void GLCommandBuffer::validateState(CommandBufferState expectedState, const char *operationName) const
{
  if (_state != expectedState)
  {
    ASSERT_TRUE(false, "GLCommandBuffer: Invalid command buffer state");
  }
}

void GLCommandBuffer::validateInRenderPass(const char *operationName) const
{
  if (_state != CommandBufferState::InRenderPass)
  {
    ASSERT_TRUE(false, "GLCommandBuffer: Command buffer must be in render pass");
  }
}

void GLCommandBuffer::validateOutsideRenderPass(const char *operationName) const
{
  if (_state == CommandBufferState::InRenderPass)
  {
    ASSERT_TRUE(false, "GLCommandBuffer: Command buffer must not be in render pass");
  }

  if (_state != CommandBufferState::Recording)
  {
    ASSERT_TRUE(false, "GLCommandBuffer: Command buffer must be in recording state");
  }
}

void GLCommandBuffer::beginDraw()
{
  ASSERT_FALSE(_boundGraphicsPipeline == nullptr, "No graphics pipeline state has been bound");
  ASSERT_FALSE(_boundGraphicsPipeline->getVS() == nullptr, "No vertex shader has been set");
  ASSERT_FALSE(_boundGraphicsPipeline->getFS() == nullptr, "No fragment shader has been set");
  ASSERT_FALSE(_device->_shaderParams == nullptr, "No shader GPU params has been set");

  if (_device->_shaderStateChanged)
  {
    auto shaderPipeline = _shaderPipelineCollection->getShaderPipeline(_boundGraphicsPipeline->getVS(),
                                                                       _boundGraphicsPipeline->getFS(),
                                                                       _boundGraphicsPipeline->getGS(),
                                                                       _boundGraphicsPipeline->getHS(),
                                                                       _boundGraphicsPipeline->getDS());

    if (_shaderPipeline == nullptr || _shaderPipeline != shaderPipeline)
    {
      glCall(glBindProgramPipeline(shaderPipeline->getId()));
      _shaderPipeline = shaderPipeline;
    }
    _device->_shaderStateChanged = false;
  }

  // If no vertex attributes are defined, allow drawing without a bound vertex buffer using an empty VAO.
  auto vertexLayout = _boundGraphicsPipeline->getVertexLayout();
  const auto &bindings = _boundGraphicsPipeline->getVertexBindings();
  const auto &attributes = _boundGraphicsPipeline->getVertexAttributes();
  const bool hasModern = !attributes.empty();
  bool hasAttributes = (vertexLayout && !vertexLayout->getDesc().empty()) || hasModern;
  if (hasAttributes)
  {
    ASSERT_FALSE(_boundVertexBuffer == nullptr, "No vertex buffer has been set");
    if (hasModern)
    {
      auto vao = GLVertexArrayObjectCollection::getVaoModern(bindings, attributes, _boundVertexBuffer);
      glCall(glBindVertexArray(vao->getId()));
    }
    else
    {
      auto vao = GLVertexArrayObjectCollection::getVao(vertexLayout, _boundVertexBuffer);
      glCall(glBindVertexArray(vao->getId()));
    }
  }
  else
  {
    static GLuint sEmptyVao = 0;
    if (sEmptyVao == 0)
    {
      glCall(glGenVertexArrays(1, &sEmptyVao));
    }
    glCall(glBindVertexArray(sEmptyVao));
  }
}

void GLCommandBuffer::endDraw()
{
  glCall(glBindVertexArray(0));
}

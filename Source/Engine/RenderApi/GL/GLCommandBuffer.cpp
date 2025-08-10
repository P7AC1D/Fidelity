#include "GLCommandBuffer.hpp"
#include "GLRenderDevice.hpp"
#include "GLRenderTarget.hpp"
#include "../../Utility/Assert.hpp"
#include "../RenderDevice.hpp"
#include "../RenderTarget.hpp"
#include "../PipelineState.hpp"
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

GLCommandBuffer::GLCommandBuffer(std::shared_ptr<GLRenderDevice> device)
    : _device(device),
      _state(CommandBufferState::Initial),
      _usage(CommandBufferUsage::OneTimeSubmit),
      _currentRenderTarget(nullptr),
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
  _currentRenderTarget = nullptr;

  // Reset rendering state owned by command buffer
  _primitiveTopology = PrimitiveTopology::TriangleList;
  _boundIndexBuffer = nullptr;
  _boundVertexBuffer = nullptr;
  _shaderPipeline = nullptr;

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
  _currentRenderTarget = nullptr;

  // Reset rendering state owned by command buffer
  _primitiveTopology = PrimitiveTopology::TriangleList;
  _boundIndexBuffer = nullptr;
  _boundVertexBuffer = nullptr;
  _shaderPipeline = nullptr;

  _hasActivePipeline = false;
  _hasActiveVertexBuffer = false;
  _hasActiveIndexBuffer = false;
}

void GLCommandBuffer::beginRenderPass(const std::shared_ptr<RenderTarget> &renderTarget,
                                      bool clearColor,
                                      bool clearDepth,
                                      bool clearStencil)
{
  validateState(CommandBufferState::Recording, "beginRenderPass");

  _currentRenderTarget = renderTarget;
  _state = CommandBufferState::InRenderPass;

  // Set the render target using command buffer method
  setRenderTarget(renderTarget);

  // Clear buffers if requested
  if (clearColor || clearDepth || clearStencil)
  {
    uint32 clearFlags = 0;
    if (clearColor)
      clearFlags |= RTT_Colour;
    if (clearDepth)
      clearFlags |= RTT_Depth;
    if (clearStencil)
      clearFlags |= RTT_Stencil;

    clearRenderTarget(clearFlags);
  }
}

void GLCommandBuffer::endRenderPass()
{
  validateState(CommandBufferState::InRenderPass, "endRenderPass");

  _state = CommandBufferState::Recording;
  _currentRenderTarget = nullptr;

  // Reset pipeline and buffer state as they're render pass specific
  _hasActivePipeline = false;
  _hasActiveVertexBuffer = false;
  _hasActiveIndexBuffer = false;
}

void GLCommandBuffer::setPipelineState(const std::shared_ptr<PipelineState> &pipelineState)
{
  validateInRenderPass("setPipelineState");

  ASSERT_FALSE(pipelineState == nullptr, "GLCommandBuffer::setPipelineState: Pipeline state cannot be null");

  // Apply primitive topology
  setPrimitiveTopology(pipelineState->getPrimitiveTopology());

  // Apply render states
  _device->setRasterizerState(pipelineState->getRasterizerState());
  _device->setDepthStencilState(pipelineState->getDepthStencilState());
  _device->setBlendState(pipelineState->getBlendState());

  // Store pipeline state and shader params in device for resource binding
  _device->_pipelineState = pipelineState;
  _device->_shaderParams = pipelineState->getShaderParams();

  // Bind existing textures to new shader
  for (uint32 i = 0; i < _device->_boundTextures.size(); i++)
  {
    if (_device->_boundTextures[i])
    {
      std::string textureName = _device->_shaderParams->getParamName(ShaderParamType::Texture, i);
      if (!textureName.empty())
      {
        auto glPs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getFS());
        if (glPs->hasUniform(textureName))
        {
          glPs->bindTextureUnit(textureName, i);
        }
      }
    }
  }

  // Bind existing constant buffers to new shader
  for (uint32 i = 0; i < _device->_boundConstantBuffers.size(); i++)
  {
    if (_device->_boundConstantBuffers[i])
    {
      auto uniformBufferName = _device->_shaderParams->getParamName(ShaderParamType::ConstBuffer, i);
      auto glVs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getVS());
      if (glVs->hasUniform(uniformBufferName))
      {
        glVs->bindUniformBlock(uniformBufferName, i);
      }

      auto glPs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getFS());
      if (glPs->hasUniform(uniformBufferName))
      {
        glPs->bindUniformBlock(uniformBufferName, i);
      }

      auto glGs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getGS());
      if (glGs && glGs->hasUniform(uniformBufferName))
      {
        glGs->bindUniformBlock(uniformBufferName, i);
      }

      auto glHs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getHS());
      if (glHs && glHs->hasUniform(uniformBufferName))
      {
        glHs->bindUniformBlock(uniformBufferName, i);
      }

      auto glDs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getDS());
      if (glDs && glDs->hasUniform(uniformBufferName))
      {
        glDs->bindUniformBlock(uniformBufferName, i);
      }
    }
  }

  _device->_shaderStateChanged = true;
  _hasActivePipeline = true;
}

void GLCommandBuffer::bindResourceSet(const std::unique_ptr<IResourceSet> &resourceSet, uint32 setIndex)
{
  validateInRenderPass("bindResourceSet");

  ASSERT_FALSE(resourceSet == nullptr, "GLCommandBuffer::bindResourceSet: Resource set cannot be null");

  // Directly handle resource set binding in command buffer
  if (resourceSet && resourceSet->isBuilt())
  {
    resourceSet->bind(_device, setIndex);

    // For OpenGL, we also need to setup uniform block bindings and texture unit bindings for the current pipeline state
    // This is necessary because resource sets bind buffers/textures to OpenGL binding points,
    // but we also need to bind uniform blocks and sampler uniforms in shaders to those same binding points
    if (_device->_pipelineState && _device->_shaderParams)
    {
      auto glResourceSet = static_cast<const GLResourceSet *>(resourceSet.get());

      // Iterate through the resource set's bindings
      for (const auto &binding : glResourceSet->getBindings())
      {
        if (binding.type == ResourceType::UNIFORM_BUFFER)
        {
          // Get the uniform buffer name from shader params
          auto uniformBufferName = _device->_shaderParams->getParamName(ShaderParamType::ConstBuffer, binding.binding);

          if (!uniformBufferName.empty())
          {
            // Bind uniform blocks in all shader stages
            auto glVs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getVS());
            if (glVs && glVs->hasUniform(uniformBufferName))
            {
              glVs->bindUniformBlock(uniformBufferName, binding.binding);
            }

            auto glPs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getFS());
            if (glPs && glPs->hasUniform(uniformBufferName))
            {
              glPs->bindUniformBlock(uniformBufferName, binding.binding);
            }

            auto glGs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getGS());
            if (glGs && glGs->hasUniform(uniformBufferName))
            {
              glGs->bindUniformBlock(uniformBufferName, binding.binding);
            }

            auto glHs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getHS());
            if (glHs && glHs->hasUniform(uniformBufferName))
            {
              glHs->bindUniformBlock(uniformBufferName, binding.binding);
            }

            auto glDs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getDS());
            if (glDs && glDs->hasUniform(uniformBufferName))
            {
              glDs->bindUniformBlock(uniformBufferName, binding.binding);
            }
          }
        }
        else if (binding.type == ResourceType::TEXTURE_2D || binding.type == ResourceType::TEXTURE_CUBE)
        {
          // Get the texture name from shader params
          auto textureName = _device->_shaderParams->getParamName(ShaderParamType::Texture, binding.binding);

          if (!textureName.empty())
          {
            // Bind texture units in all shader stages
            auto glVs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getVS());
            if (glVs && glVs->hasUniform(textureName))
            {
              glVs->bindTextureUnit(textureName, binding.binding);
            }

            auto glPs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getFS());
            if (glPs && glPs->hasUniform(textureName))
            {
              glPs->bindTextureUnit(textureName, binding.binding);
            }

            auto glGs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getGS());
            if (glGs && glGs->hasUniform(textureName))
            {
              glGs->bindTextureUnit(textureName, binding.binding);
            }

            auto glHs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getHS());
            if (glHs && glHs->hasUniform(textureName))
            {
              glHs->bindTextureUnit(textureName, binding.binding);
            }

            auto glDs = std::static_pointer_cast<GLShader>(_device->_pipelineState->getDS());
            if (glDs && glDs->hasUniform(textureName))
            {
              glDs->bindTextureUnit(textureName, binding.binding);
            }
          }
        }
      }
    }
  }
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
  ASSERT_TRUE(_hasActiveVertexBuffer, "GLCommandBuffer::draw: No vertex buffer bound");

  if (instanceCount != 1)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::draw: OpenGL implementation doesn't support instanced rendering in this path");
  }

  if (firstInstance != 0)
  {
    ASSERT_TRUE(false, "GLCommandBuffer::draw: OpenGL implementation doesn't support firstInstance parameter");
  }

  // Execute the draw call directly in command buffer
  beginDraw();
  glCall(glDrawArrays(getPrimitiveTopology(_primitiveTopology), firstVertex, vertexCount));
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
    ASSERT_TRUE(false, "GLCommandBuffer::drawIndexed: OpenGL implementation doesn't support instanced rendering in this path");
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
  glCall(glDrawElementsBaseVertex(getPrimitiveTopology(_primitiveTopology), indexCount, idxType, reinterpret_cast<GLvoid *>(idxTypeByteCount * firstIndex), vertexOffset));
  endDraw();
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

  // OpenGL buffer copy operations would be implemented here
  // This is a placeholder for future implementation
  ASSERT_TRUE(false, "GLCommandBuffer::copyBuffer: Not yet implemented for OpenGL backend");
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

  // OpenGL buffer to texture copy operations would be implemented here
  // This is a placeholder for future implementation
  ASSERT_TRUE(false, "GLCommandBuffer::copyBufferToTexture: Not yet implemented for OpenGL backend");
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

  // OpenGL buffer update operations would be implemented here
  // This is a placeholder for future implementation
  ASSERT_TRUE(false, "GLCommandBuffer::updateBuffer: Not yet implemented for OpenGL backend");
}

void GLCommandBuffer::memoryBarrier(uint32 srcStage, uint32 dstStage)
{
  // Memory barriers are not typically needed in OpenGL due to its immediate mode nature
  // However, for compute shaders or image operations, glMemoryBarrier might be needed
  // This is a no-op for basic rendering operations
}

void GLCommandBuffer::setRenderTarget(const std::shared_ptr<RenderTarget> &renderTarget)
{
  validateInRenderPass("setRenderTarget");

  if (!renderTarget)
  {
    glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    _device->_boundRenderTarget = nullptr;
    return;
  }

  auto glRenderTarget = std::static_pointer_cast<GLRenderTarget>(renderTarget);
  glCall(glBindFramebuffer(GL_FRAMEBUFFER, glRenderTarget->getId()));
  _device->_boundRenderTarget = glRenderTarget;
}

void GLCommandBuffer::clearRenderTarget(uint32 buffers, const Colour &colour, float32 depth, int32 stencil)
{
  validateInRenderPass("clearRenderTarget");

  if (!_device->_pipelineState)
  {
    return;
  }

  auto currentScissorDimensions = _device->getScissorDimensions();
  if (_device->_pipelineState->getRasterizerState()->isScissorEnabled())
  {
    ScissorDesc scissorDesc;
    scissorDesc.X = 0;
    scissorDesc.Y = 0;
    scissorDesc.H = _device->getRenderHeight();
    scissorDesc.W = _device->getRenderWidth();
    // Directly set OpenGL scissor state for clear operation
    glCall(glScissor(scissorDesc.X, scissorDesc.Y, scissorDesc.W, scissorDesc.H));
  }

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

  if (_device->_pipelineState->getRasterizerState()->isScissorEnabled())
  {
    // Restore original scissor state after clear operation
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
  ASSERT_FALSE(_device->_pipelineState == nullptr, "No pipeline state has been set");
  ASSERT_FALSE(_device->_pipelineState->getVS() == nullptr, "No vertex shader has been set");
  ASSERT_FALSE(_device->_pipelineState->getFS() == nullptr, "No pixel shader has been set");
  ASSERT_FALSE(_device->_shaderParams == nullptr, "No shader GPU params has been set");
  ASSERT_FALSE(_boundVertexBuffer == nullptr, "No vertex buffer has been set");

  if (_device->_shaderStateChanged)
  {
    auto shaderPipeline = _shaderPipelineCollection->getShaderPipeline(_device->_pipelineState->getVS(),
                                                                       _device->_pipelineState->getFS(),
                                                                       _device->_pipelineState->getGS(),
                                                                       _device->_pipelineState->getHS(),
                                                                       _device->_pipelineState->getDS());

    if (_shaderPipeline == nullptr || _shaderPipeline != shaderPipeline)
    {
      glCall(glBindProgramPipeline(shaderPipeline->getId()));
      _shaderPipeline = shaderPipeline;
    }
    _device->_shaderStateChanged = false;
  }

  auto vao = GLVertexArrayObjectCollection::getVao(_device->_pipelineState->getVertexLayout(), _boundVertexBuffer);
  glCall(glBindVertexArray(vao->getId()));
}

void GLCommandBuffer::endDraw()
{
  glCall(glBindVertexArray(0));
}

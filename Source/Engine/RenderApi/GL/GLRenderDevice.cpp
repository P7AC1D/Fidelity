#include "GLRenderDevice.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"
#include "GLCommon.hpp"
#include "GLGpuBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLRenderTarget.hpp"
#include "GLResourceSet.hpp"
#include "GLSamplerState.hpp"
#include "GLShader.hpp"
#include "GLShaderPipeline.hpp"
#include "GLShaderPipelineCollection.hpp"
#include "GLTexture.hpp"
#include "GLVertexBuffer.hpp"
#include "GLVertexArrayCollection.hpp"

GLRenderDevice::GLRenderDevice(const RenderDeviceDesc &desc) : RenderDevice(desc),
                                                               _shaderStateChanged(true),
                                                               _primitiveTopology(PrimitiveTopology::TriangleList),
                                                               _stencilReadMask(0),
                                                               _stencilRefValue(0),
                                                               _stencilWriteMask(0),
                                                               _shaderPipelineCollection(new GLShaderPipelineCollection),
                                                               _resourceSetFactory(std::make_unique<GLResourceSetFactory>())
{
  setViewport(ViewportDesc{0.0f, 0.0f, static_cast<float32>(desc.RenderWidth), static_cast<float32>(desc.RenderHeight), 0.0f, 0.0f});
  setScissorDimensions(ScissorDesc{0, 0, desc.RenderWidth, desc.RenderHeight});
}

std::shared_ptr<Shader> GLRenderDevice::createShader(const ShaderDesc &desc)
{
  std::shared_ptr<GLShader> glShader(new GLShader(desc));
  glShader->compile();
  return glShader;
}

std::shared_ptr<VertexBuffer> GLRenderDevice::createVertexBuffer(const VertexBufferDesc &desc)
{
  return std::make_shared<GLVertexBuffer>(desc);
}

std::shared_ptr<RenderTarget> GLRenderDevice::createRenderTarget(const RenderTargetDesc &desc)
{
  return std::shared_ptr<GLRenderTarget>(new GLRenderTarget(desc));
}

std::shared_ptr<IndexBuffer> GLRenderDevice::createIndexBuffer(const IndexBufferDesc &desc)
{
  return std::make_shared<GLIndexBuffer>(desc);
}

std::shared_ptr<GpuBuffer> GLRenderDevice::createGpuBuffer(const GpuBufferDesc &desc)
{
  return std::shared_ptr<GLGpuBuffer>(new GLGpuBuffer(desc));
}

std::shared_ptr<Texture> GLRenderDevice::createTexture(const TextureDesc &desc, bool gammaCorrected)
{
  return std::shared_ptr<GLTexture>(new GLTexture(desc, gammaCorrected));
}

std::shared_ptr<SamplerState> GLRenderDevice::createSamplerState(const SamplerStateDesc &desc)
{
  return std::shared_ptr<GLSamplerState>(new GLSamplerState(desc));
}

std::unique_ptr<IResourceSetLayout> GLRenderDevice::createResourceSetLayout()
{
  return _resourceSetFactory->createLayout();
}

std::unique_ptr<IResourceSet> GLRenderDevice::createResourceSet(const std::unique_ptr<IResourceSetLayout> &layout)
{
  return _resourceSetFactory->createResourceSet(layout);
}

void GLRenderDevice::setPrimitiveTopology(PrimitiveTopology primitiveTopology)
{
  _primitiveTopology = primitiveTopology;
}

void GLRenderDevice::setViewport(const ViewportDesc &viewport)
{
  if (_viewportDesc.TopLeftX != viewport.TopLeftX ||
      _viewportDesc.TopLeftY != viewport.TopLeftY ||
      _viewportDesc.Width != viewport.Width ||
      _viewportDesc.Height != viewport.Height)
  {
    glCall(glViewport(viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height));
    _viewportDesc = viewport;
  }
}

void GLRenderDevice::setPipelineState(const std::shared_ptr<PipelineState> &pipelineState)
{
  setPrimitiveTopology(pipelineState->getPrimitiveTopology());
  setRasterizerState(pipelineState->getRasterizerState());
  setDepthStencilState(pipelineState->getDepthStencilState());
  setBlendState(pipelineState->getBlendState());
  _pipelineState = pipelineState;
  _shaderParams = pipelineState->getShaderParams();

  for (uint32 i = 0; i < _boundTextures.size(); i++)
  {
    if (_boundTextures[i])
    {
      std::string textureName = _shaderParams->getParamName(ShaderParamType::Texture, i);
      if (!textureName.empty())
      {
        auto glPs = std::static_pointer_cast<GLShader>(_pipelineState->getFS());
        if (glPs->hasUniform(textureName))
        {
          glPs->bindTextureUnit(textureName, i);
        }
      }
    }
  }

  for (uint32 i = 0; i < _boundConstantBuffers.size(); i++)
  {
    if (_boundConstantBuffers[i])
    {
      auto uniformBufferName = _shaderParams->getParamName(ShaderParamType::ConstBuffer, i);
      auto glVs = std::static_pointer_cast<GLShader>(_pipelineState->getVS());
      if (glVs->hasUniform(uniformBufferName))
      {
        glVs->bindUniformBlock(uniformBufferName, i);
      }

      auto glPs = std::static_pointer_cast<GLShader>(_pipelineState->getFS());
      if (glPs->hasUniform(uniformBufferName))
      {
        glPs->bindUniformBlock(uniformBufferName, i);
      }

      auto glGs = std::static_pointer_cast<GLShader>(_pipelineState->getGS());
      if (glGs && glGs->hasUniform(uniformBufferName))
      {
        glGs->bindUniformBlock(uniformBufferName, i);
      }

      auto glHs = std::static_pointer_cast<GLShader>(_pipelineState->getHS());
      if (glHs && glHs->hasUniform(uniformBufferName))
      {
        glHs->bindUniformBlock(uniformBufferName, i);
      }

      auto glDs = std::static_pointer_cast<GLShader>(_pipelineState->getDS());
      if (glDs && glDs->hasUniform(uniformBufferName))
      {
        glDs->bindUniformBlock(uniformBufferName, i);
      }
    }
  }
  _shaderStateChanged = true;
}

void GLRenderDevice::setRenderTarget(const std::shared_ptr<RenderTarget> &renderTarget)
{
  if (!renderTarget)
  {
    glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    _boundRenderTarget = nullptr;
    return;
  }

  auto glRenderTarget = std::static_pointer_cast<GLRenderTarget>(renderTarget);
  glCall(glBindFramebuffer(GL_FRAMEBUFFER, glRenderTarget->getId()));
  _boundRenderTarget = glRenderTarget;
}

void GLRenderDevice::setVertexBuffer(const std::shared_ptr<VertexBuffer> vertexBuffer)
{
  auto glVertexBuffer = std::static_pointer_cast<GLVertexBuffer>(vertexBuffer);
  _boundVertexBuffer = glVertexBuffer;
}

void GLRenderDevice::setIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)
{
  auto glIndexBuffer = std::static_pointer_cast<GLIndexBuffer>(indexBuffer);
  _boundIndexBuffer = glIndexBuffer;
}

void GLRenderDevice::setScissorDimensions(const ScissorDesc &desc)
{
  ASSERT_FALSE(desc.X < 0.0f || desc.X > _desc.RenderWidth, "Scissor X-Pos exceeds render dimensions");
  ASSERT_FALSE(desc.Y < 0.0f || desc.Y > _desc.RenderHeight, "Scissor Y-Pos exceeds render dimensions");
  ASSERT_FALSE(desc.W < 0.0f || desc.W > _desc.RenderWidth, "Scissor width exceeds render dimensions");
  ASSERT_FALSE(desc.H < 0.0f || desc.H > _desc.RenderHeight, "Scissor height exceeds render dimensions");

  if (desc.X != _scissorDesc.X || desc.Y != _scissorDesc.Y || desc.W != _scissorDesc.W || desc.H != _scissorDesc.H)
  {
    glCall(glScissor(desc.X, desc.Y, desc.W, desc.H));
    _scissorDesc = desc;
  }
}

const ViewportDesc &GLRenderDevice::getViewport() const
{
  return _viewportDesc;
}

ScissorDesc GLRenderDevice::getScissorDimensions() const
{
  return _scissorDesc;
}

void GLRenderDevice::draw(uint32 vertexCount, uint32 vertexOffset)
{
  beginDraw();
  glCall(glDrawArrays(getPrimitiveTopology(_primitiveTopology), vertexOffset, vertexCount));
  endDraw();
}

void GLRenderDevice::drawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset)
{
  beginDraw();
  ASSERT_FALSE(_boundIndexBuffer == nullptr, "No index buffer has been bound");
  glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _boundIndexBuffer->getId()));

  GLenum idxType = _boundIndexBuffer->getIndexType() == IndexType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
  uint32 idxTypeByteCount = IndexBuffer::getBytesPerIndex(_boundIndexBuffer->getIndexType());
  glCall(glDrawElementsBaseVertex(getPrimitiveTopology(_primitiveTopology), indexCount, idxType, reinterpret_cast<GLvoid *>(idxTypeByteCount * indexOffset), vertexOffset));
  endDraw();
}

void GLRenderDevice::clearBuffers(uint32 buffers, const Colour &colour, float32 depth, int32 stencil)
{
  if (!_pipelineState)
  {
    return;
  }

  auto currentScissorDimensions = getScissorDimensions();
  if (_pipelineState->getRasterizerState()->isScissorEnabled())
  {
    ScissorDesc scissorDesc;
    scissorDesc.X = 0;
    scissorDesc.Y = 0;
    scissorDesc.H = getRenderHeight();
    scissorDesc.W = getRenderWidth();
    setScissorDimensions(scissorDesc);
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

  if (_pipelineState->getRasterizerState()->isScissorEnabled())
  {
    setScissorDimensions(currentScissorDimensions);
  }
}

void GLRenderDevice::beginDraw()
{
  ASSERT_FALSE(_pipelineState == nullptr, "No pipeline state has been set");
  ASSERT_FALSE(_pipelineState->getVS() == nullptr, "No vertex shader has been set");
  ASSERT_FALSE(_pipelineState->getFS() == nullptr, "No pixel shader has been set");
  ASSERT_FALSE(_shaderParams == nullptr, "No shader GPU params has been set");
  ASSERT_FALSE(_boundVertexBuffer == nullptr, "No vertex buffer has been set");

  if (_shaderStateChanged)
  {
    auto shaderPipeline = _shaderPipelineCollection->getShaderPipeline(_pipelineState->getVS(),
                                                                       _pipelineState->getFS(),
                                                                       _pipelineState->getGS(),
                                                                       _pipelineState->getHS(),
                                                                       _pipelineState->getDS());

    if (_shaderPipeline == nullptr || _shaderPipeline != shaderPipeline)
    {
      glCall(glBindProgramPipeline(shaderPipeline->getId()));
      _shaderPipeline = shaderPipeline;
    }
    _shaderStateChanged = false;
  }

  auto vao = GLVertexArrayObjectCollection::getVao(_pipelineState->getVertexLayout(), _boundVertexBuffer);
  glCall(glBindVertexArray(vao->getId()));
}

void GLRenderDevice::endDraw()
{
  glCall(glBindVertexArray(0));
}

void GLRenderDevice::setRasterizerState(const std::shared_ptr<RasterizerState> &rasterizerState)
{
  auto newRasterizerStateDesc = rasterizerState->getDesc();
  if (!_rasterizerState)
  {
    setDepthBias(newRasterizerStateDesc.DepthBias, newRasterizerStateDesc.SlopeScaledDepthBias);
    setCullingMode(newRasterizerStateDesc.CullMode);
    setFillMode(newRasterizerStateDesc.FillMode);
    enableScissorTest(newRasterizerStateDesc.ScissorEnabled);
    enableMultisampling(newRasterizerStateDesc.MultisampleEnabled);
    enableDepthClip(newRasterizerStateDesc.DepthClipEnabled);
    enableAntialiasedLine(newRasterizerStateDesc.AntialiasedLineEnable);

    _rasterizerState = rasterizerState;
    return;
  }

  auto oldRasterizerStateDesc = _rasterizerState->getDesc();
  if (oldRasterizerStateDesc.DepthBias != newRasterizerStateDesc.DepthBias || oldRasterizerStateDesc.SlopeScaledDepthBias != newRasterizerStateDesc.SlopeScaledDepthBias)
  {
    setDepthBias(newRasterizerStateDesc.DepthBias, newRasterizerStateDesc.SlopeScaledDepthBias);
  }

  if (oldRasterizerStateDesc.CullMode != newRasterizerStateDesc.CullMode)
  {
    setCullingMode(newRasterizerStateDesc.CullMode);
  }

  if (oldRasterizerStateDesc.FillMode != newRasterizerStateDesc.FillMode)
  {
    setFillMode(newRasterizerStateDesc.FillMode);
  }

  if (oldRasterizerStateDesc.ScissorEnabled != newRasterizerStateDesc.ScissorEnabled)
  {
    enableScissorTest(newRasterizerStateDesc.ScissorEnabled);
  }

  if (oldRasterizerStateDesc.MultisampleEnabled != newRasterizerStateDesc.MultisampleEnabled)
  {
    enableMultisampling(newRasterizerStateDesc.MultisampleEnabled);
  }

  if (oldRasterizerStateDesc.DepthClipEnabled != newRasterizerStateDesc.DepthClipEnabled)
  {
    enableDepthClip(newRasterizerStateDesc.DepthClipEnabled);
  }

  if (oldRasterizerStateDesc.AntialiasedLineEnable != newRasterizerStateDesc.AntialiasedLineEnable)
  {
    enableAntialiasedLine(newRasterizerStateDesc.AntialiasedLineEnable);
  }
  _rasterizerState = rasterizerState;
}

void GLRenderDevice::setDepthStencilState(const std::shared_ptr<DepthStencilState> &depthStencilState)
{
  auto newDepthStencilStateDesc = depthStencilState->getDesc();
  if (_depthStencilState)
  {
    auto oldDepthStencilStateDesc = _depthStencilState->getDesc();
    if (oldDepthStencilStateDesc.StencilEnabled != newDepthStencilStateDesc.StencilEnabled)
    {
      enableStencilTest(newDepthStencilStateDesc.StencilEnabled);
    }

    if (oldDepthStencilStateDesc.FrontFace.FailOp != newDepthStencilStateDesc.FrontFace.FailOp ||
        oldDepthStencilStateDesc.FrontFace.PassOp != newDepthStencilStateDesc.FrontFace.PassOp ||
        oldDepthStencilStateDesc.FrontFace.ZFailOp != newDepthStencilStateDesc.FrontFace.ZFailOp)
    {
      setStencilOperations(newDepthStencilStateDesc.FrontFace, true);
    }

    if (oldDepthStencilStateDesc.BackFace.FailOp != newDepthStencilStateDesc.BackFace.FailOp ||
        oldDepthStencilStateDesc.BackFace.PassOp != newDepthStencilStateDesc.BackFace.PassOp ||
        oldDepthStencilStateDesc.BackFace.ZFailOp != newDepthStencilStateDesc.BackFace.ZFailOp)
    {
      setStencilOperations(newDepthStencilStateDesc.BackFace, false);
    }

    if (oldDepthStencilStateDesc.FrontFace.ComparisonFunc != newDepthStencilStateDesc.FrontFace.ComparisonFunc ||
        oldDepthStencilStateDesc.StencilReadMask != newDepthStencilStateDesc.StencilReadMask)
    {
      setStencilFunction(newDepthStencilStateDesc.FrontFace.ComparisonFunc, newDepthStencilStateDesc.StencilReadMask, true);
    }

    if (oldDepthStencilStateDesc.BackFace.ComparisonFunc != newDepthStencilStateDesc.FrontFace.ComparisonFunc ||
        oldDepthStencilStateDesc.StencilReadMask != newDepthStencilStateDesc.StencilReadMask)
    {
      setStencilFunction(newDepthStencilStateDesc.BackFace.ComparisonFunc, newDepthStencilStateDesc.StencilReadMask, false);
    }

    if (oldDepthStencilStateDesc.StencilWriteMask != newDepthStencilStateDesc.StencilWriteMask)
    {
      setStencilWriteMask(newDepthStencilStateDesc.StencilWriteMask);
    }

    if (oldDepthStencilStateDesc.DepthReadEnabled != newDepthStencilStateDesc.DepthReadEnabled)
    {
      enableDepthTest(newDepthStencilStateDesc.DepthReadEnabled);
    }

    if (oldDepthStencilStateDesc.DepthWriteEnabled != newDepthStencilStateDesc.DepthWriteEnabled)
    {
      enableDepthWrite(newDepthStencilStateDesc.DepthWriteEnabled);
    }

    if (oldDepthStencilStateDesc.DepthFunc != newDepthStencilStateDesc.DepthFunc)
    {
      setDepthFunction(newDepthStencilStateDesc.DepthFunc);
    }
  }
  else
  {
    enableStencilTest(newDepthStencilStateDesc.StencilEnabled);
    setStencilOperations(newDepthStencilStateDesc.FrontFace, true);
    setStencilOperations(newDepthStencilStateDesc.BackFace, false);
    setStencilFunction(newDepthStencilStateDesc.FrontFace.ComparisonFunc, newDepthStencilStateDesc.StencilReadMask, true);
    setStencilFunction(newDepthStencilStateDesc.BackFace.ComparisonFunc, newDepthStencilStateDesc.StencilReadMask, false);
    setStencilWriteMask(newDepthStencilStateDesc.StencilWriteMask);

    enableDepthTest(newDepthStencilStateDesc.DepthReadEnabled);
    enableDepthWrite(newDepthStencilStateDesc.DepthWriteEnabled);
    setDepthFunction(newDepthStencilStateDesc.DepthFunc);
  }
  _depthStencilState = depthStencilState;
}

void GLRenderDevice::setBlendState(const std::shared_ptr<BlendState> &blendState)
{
  auto newBlendStateDesc = blendState->getDesc();
  // TODO: Investigate per-RT blending in OpenGL - for now just use the first.
  auto newFirstRTBlendState = newBlendStateDesc.RTBlendState[0];
  if (!_blendState)
  {
    enableBlend(newFirstRTBlendState.BlendEnabled);
    setBlendFactors(newFirstRTBlendState.Blend.Source, newFirstRTBlendState.Blend.Destination, newFirstRTBlendState.BlendAlpha.Source, newFirstRTBlendState.BlendAlpha.Destination);
    setBlendOperation(newFirstRTBlendState.Blend.Operation, newFirstRTBlendState.BlendAlpha.Operation);
    setBlendWriteMask(newFirstRTBlendState.RTWriteMask);
    return;
  }

  auto oldBlendStateDesc = _blendState->getDesc();
  auto oldFirstRTBlendState = oldBlendStateDesc.RTBlendState[0];
  if (oldFirstRTBlendState.BlendEnabled != newFirstRTBlendState.BlendEnabled)
  {
    enableBlend(newFirstRTBlendState.BlendEnabled);
  }

  if (oldFirstRTBlendState.Blend.Source != newFirstRTBlendState.Blend.Source ||
      oldFirstRTBlendState.Blend.Destination != newFirstRTBlendState.Blend.Destination ||
      oldFirstRTBlendState.BlendAlpha.Source != newFirstRTBlendState.BlendAlpha.Source ||
      oldFirstRTBlendState.BlendAlpha.Destination != newFirstRTBlendState.BlendAlpha.Destination)
  {
    setBlendFactors(newFirstRTBlendState.Blend.Source, newFirstRTBlendState.Blend.Destination, newFirstRTBlendState.BlendAlpha.Source, newFirstRTBlendState.BlendAlpha.Destination);
  }

  if (oldFirstRTBlendState.Blend.Operation != newFirstRTBlendState.Blend.Operation ||
      oldFirstRTBlendState.BlendAlpha.Operation != newFirstRTBlendState.BlendAlpha.Operation)
  {
    setBlendOperation(newFirstRTBlendState.Blend.Operation, newFirstRTBlendState.BlendAlpha.Operation);
  }

  if (oldFirstRTBlendState.RTWriteMask != newFirstRTBlendState.RTWriteMask)
  {
    setBlendWriteMask(newFirstRTBlendState.RTWriteMask);
  }
  _blendState = blendState;
}

void GLRenderDevice::setDepthBias(float32 constantBias, float32 slopeScaleBias)
{
  if (constantBias != 0 || slopeScaleBias != 0)
  {
    glCall(glEnable(GL_POLYGON_OFFSET_FILL));
    glCall(glEnable(GL_POLYGON_OFFSET_POINT));
    glCall(glEnable(GL_POLYGON_OFFSET_LINE));

    // TODO: Research this more as I don't entirely understand it.
    float32 scaledConstantBias = -constantBias * float32((1 << 24) - 1);
    glCall(glPolygonOffset(slopeScaleBias, scaledConstantBias));
  }
  else
  {
    glCall(glDisable(GL_POLYGON_OFFSET_FILL));
    glCall(glDisable(GL_POLYGON_OFFSET_POINT));
    glCall(glDisable(GL_POLYGON_OFFSET_LINE));
  }
}

void GLRenderDevice::setCullingMode(CullMode cullMode)
{
  switch (cullMode)
  {
  case CullMode::None:
    glCall(glDisable(GL_CULL_FACE));
    break;
  case CullMode::CounterClockwise:
    glCall(glEnable(GL_CULL_FACE));
    glCall(glCullFace(GL_BACK));
    break;
  case CullMode::Clockwise:
  default:
    glCall(glEnable(GL_CULL_FACE));
    glCall(glCullFace(GL_FRONT));
    break;
  }
}

void GLRenderDevice::setFillMode(FillMode fillMode)
{
  switch (fillMode)
  {
  case FillMode::WireFrame:
    glCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    break;
  case FillMode::Solid:
  default:
    glCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    break;
  }
}

void GLRenderDevice::setStencilOperations(const StencilOperationDesc &stencilOperationDesc, bool isFrontFace)
{
  if (isFrontFace)
  {
    glCall(glStencilOpSeparate(GL_FRONT,
                               getStencilOp(stencilOperationDesc.FailOp),
                               getStencilOp(stencilOperationDesc.ZFailOp),
                               getStencilOp(stencilOperationDesc.PassOp)));
  }
  else
  {
    glCall(glStencilOpSeparate(GL_FRONT,
                               getStencilOp(stencilOperationDesc.FailOp, true),
                               getStencilOp(stencilOperationDesc.ZFailOp, true),
                               getStencilOp(stencilOperationDesc.PassOp, true)));
  }
}

void GLRenderDevice::setStencilFunction(ComparisonFunction comparisonFunc, uint32 readMask, bool isFrontFace)
{
  _stencilReadMask = readMask;
  if (isFrontFace)
  {
    glCall(glStencilFuncSeparate(GL_FRONT, getCompareFunc(comparisonFunc), _stencilRefValue, readMask));
  }
  else
  {
    glCall(glStencilFuncSeparate(GL_BACK, getCompareFunc(comparisonFunc), _stencilRefValue, readMask));
  }
}

void GLRenderDevice::setStencilWriteMask(uint32 writeMask)
{
  _stencilWriteMask = writeMask;
  glCall(glStencilMask(writeMask));
}

void GLRenderDevice::setDepthFunction(ComparisonFunction depthFunc)
{
  glCall(glDepthFunc(getCompareFunc(depthFunc)));
}

void GLRenderDevice::setBlendFactors(BlendFactor srcFactor, BlendFactor dstFactor, BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor)
{
  GLenum src = getBlendFactor(srcFactor);
  GLenum dst = getBlendFactor(dstFactor);
  GLenum srcAlpha = getBlendFactor(srcAlphaFactor);
  GLenum dstAlpha = getBlendFactor(dstAlphaFactor);
  glCall(glBlendFuncSeparate(src, dst, srcAlpha, dstAlpha));
}

void GLRenderDevice::setBlendOperation(BlendOperation op, BlendOperation alphaOp)
{
  GLenum func = getBlendOp(op);
  GLenum alphaFunc = getBlendOp(alphaOp);
  glCall(glBlendEquationSeparate(func, alphaFunc));
}

void GLRenderDevice::setBlendWriteMask(byte writeMask)
{
  GLboolean red = GL_FALSE;
  GLboolean green = GL_FALSE;
  GLboolean blue = GL_FALSE;
  GLboolean alpha = GL_FALSE;
  if (writeMask & COLOUR_WRITE_ENABLE_RED)
  {
    red = GL_TRUE;
  }
  if (writeMask & COLOUR_WRITE_ENABLE_GREEN)
  {
    green = GL_TRUE;
  }
  if (writeMask & COLOUR_WRITE_ENABLE_BLUE)
  {
    blue = GL_TRUE;
  }
  if (writeMask * COLOUR_WRITE_ENABLE_ALPHA)
  {
    alpha = GL_TRUE;
  }
  glCall(glColorMask(red, green, blue, alpha));
}

void GLRenderDevice::bindResourceSet(const std::unique_ptr<IResourceSet> &resourceSet, uint32 setIndex)
{
  if (resourceSet && resourceSet->isBuilt())
  {
    resourceSet->bind(shared_from_this(), setIndex);

    // For OpenGL, we also need to setup uniform block bindings and texture unit bindings for the current pipeline state
    // This is necessary because resource sets bind buffers/textures to OpenGL binding points,
    // but we also need to bind uniform blocks and sampler uniforms in shaders to those same binding points
    if (_pipelineState && _shaderParams)
    {
      auto glResourceSet = static_cast<const GLResourceSet *>(resourceSet.get());

      // Iterate through the resource set's bindings
      for (const auto &binding : glResourceSet->getBindings())
      {
        if (binding.type == ResourceType::UNIFORM_BUFFER)
        {
          // Get the uniform buffer name from shader params
          auto uniformBufferName = _shaderParams->getParamName(ShaderParamType::ConstBuffer, binding.binding);

          if (!uniformBufferName.empty())
          {
            // Bind uniform blocks in all shader stages
            auto glVs = std::static_pointer_cast<GLShader>(_pipelineState->getVS());
            if (glVs && glVs->hasUniform(uniformBufferName))
            {
              glVs->bindUniformBlock(uniformBufferName, binding.binding);
            }

            auto glPs = std::static_pointer_cast<GLShader>(_pipelineState->getFS());
            if (glPs && glPs->hasUniform(uniformBufferName))
            {
              glPs->bindUniformBlock(uniformBufferName, binding.binding);
            }

            auto glGs = std::static_pointer_cast<GLShader>(_pipelineState->getGS());
            if (glGs && glGs->hasUniform(uniformBufferName))
            {
              glGs->bindUniformBlock(uniformBufferName, binding.binding);
            }

            auto glHs = std::static_pointer_cast<GLShader>(_pipelineState->getHS());
            if (glHs && glHs->hasUniform(uniformBufferName))
            {
              glHs->bindUniformBlock(uniformBufferName, binding.binding);
            }

            auto glDs = std::static_pointer_cast<GLShader>(_pipelineState->getDS());
            if (glDs && glDs->hasUniform(uniformBufferName))
            {
              glDs->bindUniformBlock(uniformBufferName, binding.binding);
            }
          }
        }
        else if (binding.type == ResourceType::TEXTURE_2D || binding.type == ResourceType::TEXTURE_CUBE)
        {
          // Get the texture name from shader params
          auto textureName = _shaderParams->getParamName(ShaderParamType::Texture, binding.binding);

          if (!textureName.empty())
          {
            // Bind texture units in all shader stages
            auto glVs = std::static_pointer_cast<GLShader>(_pipelineState->getVS());
            if (glVs && glVs->hasUniform(textureName))
            {
              glVs->bindTextureUnit(textureName, binding.binding);
            }

            auto glPs = std::static_pointer_cast<GLShader>(_pipelineState->getFS());
            if (glPs && glPs->hasUniform(textureName))
            {
              glPs->bindTextureUnit(textureName, binding.binding);
            }

            auto glGs = std::static_pointer_cast<GLShader>(_pipelineState->getGS());
            if (glGs && glGs->hasUniform(textureName))
            {
              glGs->bindTextureUnit(textureName, binding.binding);
            }

            auto glHs = std::static_pointer_cast<GLShader>(_pipelineState->getHS());
            if (glHs && glHs->hasUniform(textureName))
            {
              glHs->bindTextureUnit(textureName, binding.binding);
            }

            auto glDs = std::static_pointer_cast<GLShader>(_pipelineState->getDS());
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

void GLRenderDevice::enableScissorTest(bool enableScissorTest)
{
  if (enableScissorTest)
  {
    glCall(glEnable(GL_SCISSOR_TEST));
  }
  else
  {
    glCall(glDisable(GL_SCISSOR_TEST));
  }
}

void GLRenderDevice::enableMultisampling(bool enableMultisampling)
{
  if (enableMultisampling)
  {
    glCall(glEnable(GL_MULTISAMPLE));
  }
  else
  {
    glCall(glDisable(GL_MULTISAMPLE));
  }
}

void GLRenderDevice::enableDepthClip(bool enableDepthClip)
{
  if (enableDepthClip)
  {
    glCall(glEnable(GL_DEPTH_CLAMP));
  }
  else
  {
    glCall(glDisable(GL_DEPTH_CLAMP));
  }
}

void GLRenderDevice::enableAntialiasedLine(bool enableAntialiasedLine)
{
  if (enableAntialiasedLine)
  {
    glCall(glEnable(GL_LINE_SMOOTH));
  }
  else
  {
    glCall(glDisable(GL_LINE_SMOOTH));
  }
}

void GLRenderDevice::enableStencilTest(bool enableStencilTest)
{
  if (enableStencilTest)
  {
    glCall(glEnable(GL_STENCIL_TEST));
  }
  else
  {
    glCall(glDisable(GL_STENCIL_TEST));
  }
}

void GLRenderDevice::enableDepthTest(bool enableDepthTest)
{
  if (enableDepthTest)
  {
    glCall(glEnable(GL_DEPTH_TEST));
  }
  else
  {
    glCall(glDisable(GL_DEPTH_TEST));
  }
}

void GLRenderDevice::enableDepthWrite(bool enableDepthWrite)
{
  if (enableDepthWrite)
  {
    glCall(glDepthMask(GL_TRUE));
  }
  else
  {
    glCall(glDepthMask(GL_FALSE));
  }
}

void GLRenderDevice::enableBlend(bool enableBlend)
{
  if (enableBlend)
  {
    glCall(glEnable(GL_BLEND));
  }
  else
  {
    glCall(glDisable(GL_BLEND));
  }
}

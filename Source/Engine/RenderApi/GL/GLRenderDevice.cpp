#include "GLRenderDevice.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"
#include "GLCommon.hpp"
#include "GLCommandBuffer.hpp"
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
                                                               _stencilReadMask(0),
                                                               _stencilRefValue(0),
                                                               _stencilWriteMask(0),
                                                               _shaderPipelineCollection(new GLShaderPipelineCollection),
                                                               _resourceSetFactory(std::make_unique<GLResourceSetFactory>())
{
  // Initialize viewport and scissor state directly
  _viewportDesc = ViewportDesc{0.0f, 0.0f, static_cast<float32>(desc.RenderWidth), static_cast<float32>(desc.RenderHeight), 0.0f, 1.0f};
  _scissorDesc = ScissorDesc{0, 0, desc.RenderWidth, desc.RenderHeight};

  // Set OpenGL state to match initial values
  glCall(glViewport(_viewportDesc.TopLeftX, _viewportDesc.TopLeftY, _viewportDesc.Width, _viewportDesc.Height));
  glCall(glScissor(_scissorDesc.X, _scissorDesc.Y, _scissorDesc.W, _scissorDesc.H));
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

std::unique_ptr<ICommandBuffer> GLRenderDevice::createCommandBuffer()
{
  return std::make_unique<GLCommandBuffer>(shared_from_this());
}

const ViewportDesc &GLRenderDevice::getViewport() const
{
  return _viewportDesc;
}

ScissorDesc GLRenderDevice::getScissorDimensions() const
{
  return _scissorDesc;
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
  if (writeMask & COLOUR_WRITE_ENABLE_ALPHA)
  {
    alpha = GL_TRUE;
  }
  glCall(glColorMask(red, green, blue, alpha));
}

// Resource binding logic migrated to GLCommandBuffer

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

#include "GLRenderDevice.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"
#include "GLGpuBuffer.hpp"
#include "GLIndexBuffer.hpp"
#include "GLRenderTarget.hpp"
#include "GLSamplerState.hpp"
#include "GLShader.hpp"
#include "GLShaderPipeline.hpp"
#include "GLShaderPipelineCollection.hpp"
#include "GLTexture.hpp"
#include "GLVertexBuffer.hpp"
#include "GLVertexArrayCollection.hpp"

GLenum GetTextureTargetFromType(TextureType textureType)
{
  switch(textureType)
  {
    case TextureType::Texture1D: return GL_TEXTURE_1D;
    case TextureType::Texture1DArray: return GL_TEXTURE_1D_ARRAY;
    case TextureType::Texture2D: return GL_TEXTURE_2D;
    case TextureType::Texture2DArray: return GL_TEXTURE_2D_ARRAY;
    case TextureType::Texture3D: return GL_TEXTURE_3D;
    case TextureType::TextureCube: return GL_TEXTURE_CUBE_MAP;
    default: return GL_TEXTURE_2D;
  }
}

GLenum GetPrimitiveTopology(PrimitiveTopology topology)
{
  switch (topology)
  {
    case PrimitiveTopology::PointList: return GL_POINTS;
    case PrimitiveTopology::LineList: return GL_LINE;
    case PrimitiveTopology::TriangleList: return GL_TRIANGLES;
    case PrimitiveTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
    case PrimitiveTopology::PatchList: return GL_PATCHES;
    default: GL_TRIANGLES;
  }
}

GLenum GetStencilOp(StencilOperation stencilOperation, bool invert = false)
{
  switch (stencilOperation)
  {
    case StencilOperation::Keep: return GL_KEEP;
    case StencilOperation::Zero: return GL_ZERO;
    case StencilOperation::Replace: return GL_REPLACE;
    case StencilOperation::Incr: return invert ? GL_DECR : GL_INCR;
    case StencilOperation::Decr: return invert ? GL_INCR : GL_DECR;
    case StencilOperation::IncrSat: return invert ? GL_DECR_WRAP : GL_INCR_WRAP;
    case StencilOperation::DescSat: return invert ? GL_INCR_WRAP: GL_DECR_WRAP;
    case StencilOperation::Invert: return GL_INVERT;
    default: return GL_KEEP;
  }
}

GLenum GetCompareFunc(ComparisonFunction func)
{
  switch (func)
  {
    case ComparisonFunction::Never: return GL_NEVER;
    case ComparisonFunction::Less: return GL_LESS;
    case ComparisonFunction::Equal: return GL_EQUAL;
    case ComparisonFunction::LessEqual: return GL_LEQUAL;
    case ComparisonFunction::Greater: return GL_GREATER;
    case ComparisonFunction::NotEqual: return GL_NOTEQUAL;
    case ComparisonFunction::GreaterEqual: return GL_GEQUAL;
    case ComparisonFunction::Always: return GL_ALWAYS;
    default: return GL_ALWAYS;
  }
}

GLenum GetBlendFactor(BlendFactor factor)
{
  switch (factor)
  {
    case BlendFactor::Zero: return GL_ZERO;
    case BlendFactor::One: return GL_ONE;
    case BlendFactor::SrcColour: return GL_SRC_COLOR;
    case BlendFactor::InvSrcColour: return GL_ONE_MINUS_SRC_COLOR;
    case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
    case BlendFactor::InvSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DestAlpha: return GL_DST_ALPHA;
    case BlendFactor::InvDestAlpha: return GL_ONE_MINUS_DST_ALPHA;
    case BlendFactor::DestColour: return GL_DST_COLOR;
    case BlendFactor::InvDestColour: return GL_ONE_MINUS_DST_COLOR;
    default: return GL_ONE;
  }
}

GLenum GetBlendOp(BlendOperation op)
{
  switch (op)
  {
    case BlendOperation::Add: return GL_FUNC_ADD;
    case BlendOperation::Subtract: return GL_FUNC_SUBTRACT;
    case BlendOperation::RevSubtract: return GL_FUNC_REVERSE_SUBTRACT;
    case BlendOperation::Min: return GL_MIN;
    case BlendOperation::Max: return GL_MAX;
    default: return GL_FUNC_ADD;
  }
}

GLRenderDevice::GLRenderDevice(const RenderDeviceDesc& desc) :
  RenderDevice(desc),
	_textureStateChanged(true),
	_constBufferStateChanged(true),
	_shaderStateChanged(true),
  _primitiveTopology(PrimitiveTopology::TriangleList),
  _stencilReadMask(0),
  _stencilRefValue(0),
  _stencilWriteMask(0),
  _shaderPipelineCollection(new GLShaderPipelineCollection),
	_vaoCollection(new GLVertexArrayObjectCollection)
{
#ifdef _WIN32
  glewExperimental = GL_TRUE;
  GLenum error = glewInit();
  ASSERT_FALSE(error == GLEW_OK, "Failed to initialize GLEW");
#endif

	SetViewport(ViewportDesc{ 0.0f, 0.0f, static_cast<float32>(desc.RenderWidth), static_cast<float32>(desc.RenderHeight), 0.0f, 0.0f });
	SetScissorDimensions(ScissorDesc{ 0, 0, desc.RenderWidth, desc.RenderHeight });
}

std::shared_ptr<Shader> GLRenderDevice::CreateShader(const ShaderDesc& desc)
{
	std::shared_ptr<GLShader> glShader(new GLShader(desc));
	glShader->Compile();
	return glShader;
}

std::shared_ptr<VertexBuffer> GLRenderDevice::CreateVertexBuffer(const VertexBufferDesc& desc)
{
  return std::shared_ptr<GLVertexBuffer>(new GLVertexBuffer(desc));
}

std::shared_ptr<RenderTarget> GLRenderDevice::CreateRenderTarget(const RenderTargetDesc& desc)
{
  return std::shared_ptr<GLRenderTarget>(new GLRenderTarget(desc));
}

std::shared_ptr<IndexBuffer> GLRenderDevice::CreateIndexBuffer(const IndexBufferDesc& desc)
{
  return std::shared_ptr<GLIndexBuffer>(new GLIndexBuffer(desc));
}

std::shared_ptr<GpuBuffer> GLRenderDevice::CreateGpuBuffer(const GpuBufferDesc& desc)
{
  return std::shared_ptr<GLGpuBuffer>(new GLGpuBuffer(desc));
}

std::shared_ptr<Texture> GLRenderDevice::CreateTexture(const TextureDesc& desc)
{
  return std::shared_ptr<GLTexture>(new GLTexture(desc));
}

std::shared_ptr<SamplerState> GLRenderDevice::CreateSamplerState(const SamplerStateDesc& desc)
{
  return std::shared_ptr<GLSamplerState>(new GLSamplerState(desc));
}

void GLRenderDevice::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
{
  _primitiveTopology = primitiveTopology;
}

void GLRenderDevice::SetViewport(const ViewportDesc& viewport)
{
	if (_viewportDesc.TopLeftX != viewport.TopLeftX ||
			_viewportDesc.TopLeftY != viewport.TopLeftY ||
			_viewportDesc.Width != viewport.Width ||
			_viewportDesc.Height != viewport.Height)
	{
		GLCall(glViewport(viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height));
		_viewportDesc = viewport;
	}
}

void GLRenderDevice::SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState)
{
  SetRasterizerState(pipelineState->GetRasterizerState());
  SetDepthStencilState(pipelineState->GetDepthStencilState());
	SetBlendState(pipelineState->GetBlendState());
  _pipelineState = pipelineState;
  _shaderParams = pipelineState->GetShaderParams();
	_shaderStateChanged = true;
}

void GLRenderDevice::SetRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget)
{
	if (!renderTarget)
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		_boundRenderTarget = nullptr;
		return;
	}

  auto glRenderTarget = std::static_pointer_cast<GLRenderTarget>(renderTarget);
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, glRenderTarget->GetId()));
	_boundRenderTarget = glRenderTarget;
}

void GLRenderDevice::SetVertexBuffer(uint32 slot, const std::shared_ptr<VertexBuffer> vertexBuffer)
{
  auto glVertexBuffer = std::static_pointer_cast<GLVertexBuffer>(vertexBuffer);
  _boundVertexBuffers[slot] = glVertexBuffer;
}

void GLRenderDevice::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
  auto glIndexBuffer = std::static_pointer_cast<GLIndexBuffer>(indexBuffer);
	_boundIndexBuffer = glIndexBuffer;
}

void GLRenderDevice::SetConstantBuffer(uint32 slot, const std::shared_ptr<GpuBuffer>& constantBuffer)
{
  ASSERT_FALSE(constantBuffer->GetType() == BufferType::Constant, "GPU buffer is not a constant buffer");
  ASSERT_TRUE(slot > MaxConstantBuffers, "Constant buffer binding slot exceeds maximum supported");
  
  auto glConstantBuffer = std::static_pointer_cast<GLGpuBuffer>(constantBuffer);
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, slot, glConstantBuffer->GetId()));
  _boundConstantBuffers[slot] = glConstantBuffer;
	_constBufferStateChanged = true;
}

void GLRenderDevice::SetTexture(uint32 slot, const std::shared_ptr<Texture>& texture)
{
  ASSERT_TRUE(slot >= MaxTextureSlots, "Texture slot exceeds maximum supported");
  auto glTexture = std::static_pointer_cast<GLTexture>(texture);
  if (!_boundTextures[slot] || _boundTextures[slot]->GetId() != glTexture->GetId())
  {
    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
    GLCall(glBindTexture(GetTextureTargetFromType(glTexture->GetTextureType()), glTexture->GetId()));
    _boundTextures[slot] = glTexture;
		_textureStateChanged = true;
  }
}

void GLRenderDevice::SetSamplerState(uint32 slot, const std::shared_ptr<SamplerState>& samplerState)
{
  ASSERT_TRUE(slot >= MaxTextureSlots, "Sampler slot exceeds maximum supported");
  auto glSamplerState = std::static_pointer_cast<GLSamplerState>(samplerState);
  if (!_boundSamplers[slot] || _boundSamplers[slot]->GetId() != glSamplerState->GetId())
  {
    GLCall(glBindSampler(slot, glSamplerState->GetId()));
    _boundSamplers[slot] = glSamplerState;
  }
}

void GLRenderDevice::SetScissorDimensions(const ScissorDesc& desc)
{
	ASSERT_TRUE(desc.X < 0.0f || desc.X > _desc.RenderWidth, "Scissor X-Pos exceeds render dimensions");
	ASSERT_TRUE(desc.Y < 0.0f || desc.Y > _desc.RenderHeight, "Scissor Y-Pos exceeds render dimensions");
	ASSERT_TRUE(desc.W < 0.0f || desc.W > _desc.RenderWidth, "Scissor width exceeds render dimensions");
	ASSERT_TRUE(desc.H < 0.0f || desc.H > _desc.RenderHeight, "Scissor height exceeds render dimensions");

	if (desc.X != _scissorDesc.X || desc.Y != _scissorDesc.Y || desc.W != _scissorDesc.W || desc.H != _scissorDesc.H)
	{
		GLCall(glScissor(desc.X, desc.Y, desc.W, desc.H));
		_scissorDesc = desc;
	}
}

const ViewportDesc& GLRenderDevice::GetViewport() const
{
	return _viewportDesc;
}

ScissorDesc GLRenderDevice::GetScissorDimensions() const
{
	return _scissorDesc;
}

void GLRenderDevice::Draw(uint32 vertexCount, uint32 vertexOffset)
{
  BeginDraw();
  GLCall(glDrawArrays(GetPrimitiveTopology(_primitiveTopology), vertexOffset, vertexCount));
  EndDraw();
}

void GLRenderDevice::DrawIndexed(uint32 indexCount, uint32 indexOffset, uint32 vertexOffset)
{
  BeginDraw();
	ASSERT_TRUE(_boundIndexBuffer == nullptr, "No index buffer has been bound");
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _boundIndexBuffer->GetId()));

	GLenum idxType = _boundIndexBuffer->GetIndexType() == IndexType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
	uint32 idxTypeByteCount = IndexBuffer::GetBytesPerIndex(_boundIndexBuffer->GetIndexType());
  GLCall(glDrawElementsBaseVertex(GetPrimitiveTopology(_primitiveTopology), indexCount, idxType, reinterpret_cast<GLvoid*>(idxTypeByteCount * indexOffset), vertexOffset));
  EndDraw();
}

void GLRenderDevice::ClearBuffers(uint32 buffers, const Colour& colour, float32 depth, int32 stencil)
{
	if (!_pipelineState)
	{
		return;
	}

	auto currentScissorDimensions = GetScissorDimensions();
	if (_pipelineState->GetRasterizerState()->IsScissorEnabled())
	{
		ScissorDesc scissorDesc;
		scissorDesc.X = 0;
		scissorDesc.Y = 0;
		scissorDesc.H = GetRenderHeight();
		scissorDesc.W = GetRenderWidth();
		SetScissorDimensions(scissorDesc);
	}

	GLbitfield flags = 0;
	if (buffers & RTT_Colour)
	{
		flags |= GL_COLOR_BUFFER_BIT;
		GLCall(glClearColor(colour[0], colour[1], colour[2], colour[3]));
	}
	if (buffers & RTT_Depth)
	{
		flags |= GL_DEPTH_BUFFER_BIT;
		GLCall(glClearDepth(depth));
	}
	if (buffers & RTT_Stencil)
	{
		flags |= GL_STENCIL_BUFFER_BIT;
		GLCall(glClearStencil(stencil));
	}
	GLCall(glClear(flags));

	if (_pipelineState->GetRasterizerState()->IsScissorEnabled())
	{
		SetScissorDimensions(currentScissorDimensions);
	}
}

void GLRenderDevice::BeginDraw()
{
  ASSERT_TRUE(_pipelineState == nullptr, "No pipeline state has been set");
  ASSERT_TRUE(_pipelineState->GetVS() == nullptr, "No vertex shader has been set");
  ASSERT_TRUE(_pipelineState->GetPS() == nullptr, "No pixel shader has been set");
  ASSERT_TRUE(_shaderParams == nullptr, "No shader GPU params has been set");

	if (_shaderStateChanged)
	{
		auto shaderPipeline = _shaderPipelineCollection->GetShaderPipeline(_pipelineState->GetVS(),
			_pipelineState->GetPS(),
			_pipelineState->GetGS(),
			_pipelineState->GetHS(),
			_pipelineState->GetDS());

		if (_shaderPipeline == nullptr || _shaderPipeline != shaderPipeline)
		{
			GLCall(glBindProgramPipeline(shaderPipeline->GetId()));
			_shaderPipeline = shaderPipeline;
		}
		_shaderStateChanged = false;
	}
  
  auto glVertexShader = std::static_pointer_cast<GLShader>(_pipelineState->GetVS());
  auto vao = _vaoCollection->GetVao(glVertexShader->GetId(), _pipelineState->GetVertexLayout(), _boundVertexBuffers);
	GLCall(glBindVertexArray(vao->GetId()));

	if (_textureStateChanged || _shaderStateChanged)
	{
		for (uint32 i = 0; i < _boundTextures.size(); i++)
		{
			if (_boundTextures[i])
			{
				std::string textureName = _shaderParams->GetParamName(ShaderParamType::Texture, i);
				if (!textureName.empty())
				{
					auto glPs = std::static_pointer_cast<GLShader>(_pipelineState->GetPS());
					if (glPs->HasUniform(textureName))
					{
						glPs->BindTextureUnit(textureName, i);
					}
				}
			}
		}
		_textureStateChanged = false;
	}
  
	if (_constBufferStateChanged || _shaderStateChanged)
	{
		for (uint32 i = 0; i < _boundConstantBuffers.size(); i++)
		{
			if (_boundConstantBuffers[i])
			{
				auto uniformBufferName = _shaderParams->GetParamName(ShaderParamType::ConstBuffer, i);
				auto glVs = std::static_pointer_cast<GLShader>(_pipelineState->GetVS());
				if (glVs->HasUniform(uniformBufferName))
				{
					glVs->BindUniformBlock(uniformBufferName, i);
				}

				auto glPs = std::static_pointer_cast<GLShader>(_pipelineState->GetPS());
				if (glPs->HasUniform(uniformBufferName))
				{
					glPs->BindUniformBlock(uniformBufferName, i);
				}

				auto glGs = std::static_pointer_cast<GLShader>(_pipelineState->GetGS());
				if (glGs && glGs->HasUniform(uniformBufferName))
				{
					glGs->BindUniformBlock(uniformBufferName, i);
				}

				auto glHs = std::static_pointer_cast<GLShader>(_pipelineState->GetHS());
				if (glHs && glHs->HasUniform(uniformBufferName))
				{
					glHs->BindUniformBlock(uniformBufferName, i);
				}

				auto glDs = std::static_pointer_cast<GLShader>(_pipelineState->GetDS());
				if (glDs && glDs->HasUniform(uniformBufferName))
				{
					glDs->BindUniformBlock(uniformBufferName, i);
				}
			}
		}
		_constBufferStateChanged = false;
	}
}

void GLRenderDevice::EndDraw()
{
  GLCall(glBindVertexArray(0));
}

void GLRenderDevice::SetRasterizerState(const std::shared_ptr<RasterizerState>& rasterizerState)
{
  auto newRasterizerStateDesc = rasterizerState->GetDesc();
  if (!_rasterizerState)
  {
    SetDepthBias(newRasterizerStateDesc.DepthBias, newRasterizerStateDesc.SlopeScaledDepthBias);
    SetCullingMode(newRasterizerStateDesc.CullMode);
    SetFillMode(newRasterizerStateDesc.FillMode);
    EnableScissorTest(newRasterizerStateDesc.ScissorEnabled);
    EnableMultisampling(newRasterizerStateDesc.MultisampleEnabled);
    EnableDepthClip(newRasterizerStateDesc.DepthClipEnabled);
    EnableAntialiasedLine(newRasterizerStateDesc.AntialiasedLineEnable);
    
    _rasterizerState = rasterizerState;
    return;
  }
  
  auto oldRasterizerStateDesc = _rasterizerState->GetDesc();
  if (oldRasterizerStateDesc.DepthBias != newRasterizerStateDesc.DepthBias || oldRasterizerStateDesc.SlopeScaledDepthBias != newRasterizerStateDesc.SlopeScaledDepthBias)
  {
    SetDepthBias(newRasterizerStateDesc.DepthBias, newRasterizerStateDesc.SlopeScaledDepthBias);
  }
  
  if (oldRasterizerStateDesc.CullMode != newRasterizerStateDesc.CullMode)
  {
    SetCullingMode(newRasterizerStateDesc.CullMode);
  }
  
  if (oldRasterizerStateDesc.FillMode != newRasterizerStateDesc.FillMode)
  {
    SetFillMode(newRasterizerStateDesc.FillMode);
  }
  
  if (oldRasterizerStateDesc.ScissorEnabled != newRasterizerStateDesc.ScissorEnabled)
  {
    EnableScissorTest(newRasterizerStateDesc.ScissorEnabled);
  }
  
  if (oldRasterizerStateDesc.MultisampleEnabled != newRasterizerStateDesc.MultisampleEnabled)
  {
    EnableMultisampling(newRasterizerStateDesc.MultisampleEnabled);
  }
  
  if (oldRasterizerStateDesc.DepthClipEnabled != newRasterizerStateDesc.DepthClipEnabled)
  {
    EnableDepthClip(newRasterizerStateDesc.DepthClipEnabled);
  }
  
  if (oldRasterizerStateDesc.AntialiasedLineEnable != newRasterizerStateDesc.AntialiasedLineEnable)
  {
    EnableAntialiasedLine(newRasterizerStateDesc.AntialiasedLineEnable);
  }
  _rasterizerState = rasterizerState;
}

void GLRenderDevice::SetDepthStencilState(const std::shared_ptr<DepthStencilState>& depthStencilState)
{
  auto newDepthStencilStateDesc = depthStencilState->GetDesc();
	if (_depthStencilState)
	{
		auto oldDepthStencilStateDesc = _depthStencilState->GetDesc();
		if (oldDepthStencilStateDesc.StencilEnabled != newDepthStencilStateDesc.StencilEnabled)
		{
			EnableStencilTest(newDepthStencilStateDesc.StencilEnabled);
		}

		if (oldDepthStencilStateDesc.FrontFace.FailOp != newDepthStencilStateDesc.FrontFace.FailOp ||
			oldDepthStencilStateDesc.FrontFace.PassOp != newDepthStencilStateDesc.FrontFace.PassOp ||
			oldDepthStencilStateDesc.FrontFace.ZFailOp != newDepthStencilStateDesc.FrontFace.ZFailOp)
		{
			SetStencilOperations(newDepthStencilStateDesc.FrontFace, true);
		}

		if (oldDepthStencilStateDesc.BackFace.FailOp != newDepthStencilStateDesc.BackFace.FailOp ||
			oldDepthStencilStateDesc.BackFace.PassOp != newDepthStencilStateDesc.BackFace.PassOp ||
			oldDepthStencilStateDesc.BackFace.ZFailOp != newDepthStencilStateDesc.BackFace.ZFailOp)
		{
			SetStencilOperations(newDepthStencilStateDesc.BackFace, false);
		}

		if (oldDepthStencilStateDesc.FrontFace.ComparisonFunc != newDepthStencilStateDesc.FrontFace.ComparisonFunc ||
			oldDepthStencilStateDesc.StencilReadMask != newDepthStencilStateDesc.StencilReadMask)
		{
			SetStencilFunction(newDepthStencilStateDesc.FrontFace.ComparisonFunc, newDepthStencilStateDesc.StencilReadMask, true);
		}

		if (oldDepthStencilStateDesc.BackFace.ComparisonFunc != newDepthStencilStateDesc.FrontFace.ComparisonFunc ||
			oldDepthStencilStateDesc.StencilReadMask != newDepthStencilStateDesc.StencilReadMask)
		{
			SetStencilFunction(newDepthStencilStateDesc.BackFace.ComparisonFunc, newDepthStencilStateDesc.StencilReadMask, false);
		}

		if (oldDepthStencilStateDesc.StencilWriteMask != newDepthStencilStateDesc.StencilWriteMask)
		{
			SetStencilWriteMask(newDepthStencilStateDesc.StencilWriteMask);
		}

		if (oldDepthStencilStateDesc.DepthReadEnabled != newDepthStencilStateDesc.DepthReadEnabled)
		{
			EnableDepthTest(newDepthStencilStateDesc.DepthReadEnabled);
		}

		if (oldDepthStencilStateDesc.DepthWriteEnabled != newDepthStencilStateDesc.DepthWriteEnabled)
		{
			EnableDepthWrite(newDepthStencilStateDesc.DepthWriteEnabled);
		}

		if (oldDepthStencilStateDesc.DepthFunc != newDepthStencilStateDesc.DepthFunc)
		{
			SetDepthFunction(newDepthStencilStateDesc.DepthFunc);
		}
	}
	else
	{
		EnableStencilTest(newDepthStencilStateDesc.StencilEnabled);
		SetStencilOperations(newDepthStencilStateDesc.FrontFace, true);
		SetStencilOperations(newDepthStencilStateDesc.BackFace, false);
		SetStencilFunction(newDepthStencilStateDesc.FrontFace.ComparisonFunc, newDepthStencilStateDesc.StencilReadMask, true);
		SetStencilFunction(newDepthStencilStateDesc.BackFace.ComparisonFunc, newDepthStencilStateDesc.StencilReadMask, false);
		SetStencilWriteMask(newDepthStencilStateDesc.StencilWriteMask);

		EnableDepthTest(newDepthStencilStateDesc.DepthReadEnabled);
		EnableDepthWrite(newDepthStencilStateDesc.DepthWriteEnabled);
		SetDepthFunction(newDepthStencilStateDesc.DepthFunc);
	}
  _depthStencilState = depthStencilState;
}

void GLRenderDevice::SetBlendState(const std::shared_ptr<BlendState>& blendState)
{
  auto newBlendStateDesc = blendState->GetDesc();
  // TODO: Investigate per-RT blending in OpenGL - for now just use the first.
  auto newFirstRTBlendState = newBlendStateDesc.RTBlendState[0];
  if (!_blendState)
  {
    EnableBlend(newFirstRTBlendState.BlendEnabled);
    SetBlendFactors(newFirstRTBlendState.Blend.Source, newFirstRTBlendState.Blend.Destination, newFirstRTBlendState.BlendAlpha.Source, newFirstRTBlendState.BlendAlpha.Destination);
    SetBlendOperation(newFirstRTBlendState.Blend.Operation, newFirstRTBlendState.BlendAlpha.Operation);
    SetBlendWriteMask(newFirstRTBlendState.RTWriteMask);
    return;
  }
  
  auto oldBlendStateDesc = _blendState->GetDesc();
  auto oldFirstRTBlendState =oldBlendStateDesc.RTBlendState[0];
  if (oldFirstRTBlendState.BlendEnabled != newFirstRTBlendState.BlendEnabled)
  {
    EnableBlend(newFirstRTBlendState.BlendEnabled);
  }
  
  if (oldFirstRTBlendState.Blend.Source != newFirstRTBlendState.Blend.Source ||
      oldFirstRTBlendState.Blend.Destination != newFirstRTBlendState.Blend.Destination ||
      oldFirstRTBlendState.BlendAlpha.Source != newFirstRTBlendState.BlendAlpha.Source ||
      oldFirstRTBlendState.BlendAlpha.Destination != newFirstRTBlendState.BlendAlpha.Destination)
  {
    SetBlendFactors(newFirstRTBlendState.Blend.Source, newFirstRTBlendState.Blend.Destination, newFirstRTBlendState.BlendAlpha.Source, newFirstRTBlendState.BlendAlpha.Destination);
  }
  
  if (oldFirstRTBlendState.Blend.Operation != newFirstRTBlendState.Blend.Operation ||
      oldFirstRTBlendState.BlendAlpha.Operation != newFirstRTBlendState.BlendAlpha.Operation)
  {
    SetBlendOperation(newFirstRTBlendState.Blend.Operation, newFirstRTBlendState.BlendAlpha.Operation);
  }
  
  if (oldFirstRTBlendState.RTWriteMask != newFirstRTBlendState.RTWriteMask)
  {
    SetBlendWriteMask(newFirstRTBlendState.RTWriteMask);
  }
  _blendState = blendState;
}

void GLRenderDevice::SetDepthBias(float32 constantBias, float32 slopeScaleBias)
{
  if (constantBias != 0 || slopeScaleBias != 0)
  {
    GLCall(glEnable(GL_POLYGON_OFFSET_FILL));
    GLCall(glEnable(GL_POLYGON_OFFSET_POINT));
    GLCall(glEnable(GL_POLYGON_OFFSET_LINE));
    
    // TODO: Research this more as I don't entirely understand it.
    float32 scaledConstantBias = -constantBias * float32((1 << 24) - 1);
    GLCall(glPolygonOffset(slopeScaleBias, scaledConstantBias));
  }
  else
  {
    GLCall(glDisable(GL_POLYGON_OFFSET_FILL));
    GLCall(glDisable(GL_POLYGON_OFFSET_POINT));
    GLCall(glDisable(GL_POLYGON_OFFSET_LINE));
  }
}

void GLRenderDevice::SetCullingMode(CullMode cullMode)
{
  switch (cullMode)
  {
    case CullMode::None:
      GLCall(glDisable(GL_CULL_FACE));
      break;
    case CullMode::CounterClockwise:
      GLCall(glEnable(GL_CULL_FACE));
      GLCall(glCullFace(GL_BACK));
      break;
    case CullMode::Clockwise:
    default:
      GLCall(glEnable(GL_CULL_FACE));
      GLCall(glCullFace(GL_FRONT));
      break;
  }
}

void GLRenderDevice::SetFillMode(FillMode fillMode)
{
  switch (fillMode)
  {
    case FillMode::WireFrame:
      GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
      break;
    case FillMode::Solid:
    default:
      GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
      break;
  }
}

void GLRenderDevice::SetStencilOperations(const StencilOperationDesc& stencilOperationDesc, bool isFrontFace)
{
  if (isFrontFace)
  {
    GLCall(glStencilOpSeparate(GL_FRONT,
                               GetStencilOp(stencilOperationDesc.FailOp),
                               GetStencilOp(stencilOperationDesc.ZFailOp),
                               GetStencilOp(stencilOperationDesc.PassOp)));
  }
  else
  {
    GLCall(glStencilOpSeparate(GL_FRONT,
                               GetStencilOp(stencilOperationDesc.FailOp, true),
                               GetStencilOp(stencilOperationDesc.ZFailOp, true),
                               GetStencilOp(stencilOperationDesc.PassOp, true)));
  }
}

void GLRenderDevice::SetStencilFunction(ComparisonFunction comparisonFunc, uint32 readMask, bool isFrontFace)
{
  _stencilReadMask = readMask;
  if (isFrontFace)
  {
    GLCall(glStencilFuncSeparate(GL_FRONT, GetCompareFunc(comparisonFunc), _stencilRefValue, readMask));
  }
  else
  {
    GLCall(glStencilFuncSeparate(GL_BACK, GetCompareFunc(comparisonFunc), _stencilRefValue, readMask));
  }
}

void GLRenderDevice::SetStencilWriteMask(uint32 writeMask)
{
  _stencilWriteMask = writeMask;
  GLCall(glStencilMask(writeMask));
}

void GLRenderDevice::SetDepthFunction(ComparisonFunction depthFunc)
{
  GLCall(glDepthFunc(GetCompareFunc(depthFunc)));
}

void GLRenderDevice::SetBlendFactors(BlendFactor srcFactor, BlendFactor dstFactor, BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor)
{
  GLenum src = GetBlendFactor(srcFactor);
  GLenum dst = GetBlendFactor(dstFactor);
  GLenum srcAlpha = GetBlendFactor(srcAlphaFactor);
  GLenum dstAlpha = GetBlendFactor(dstAlphaFactor);
  GLCall(glBlendFuncSeparate(src, dst, srcAlpha, dstAlpha));
}

void GLRenderDevice::SetBlendOperation(BlendOperation op, BlendOperation alphaOp)
{
  GLenum func = GetBlendOp(op);
  GLenum alphaFunc = GetBlendOp(alphaOp);
  GLCall(glBlendEquationSeparate(func, alphaFunc));
}

void GLRenderDevice::SetBlendWriteMask(byte writeMask)
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
  GLCall(glColorMask(red, green, blue, alpha));
}

void GLRenderDevice::EnableScissorTest(bool enableScissorTest)
{
  if (enableScissorTest)
  {
    GLCall(glEnable(GL_SCISSOR_TEST));
  }
  else
  {
    GLCall(glDisable(GL_SCISSOR_TEST));
  }
}

void GLRenderDevice::EnableMultisampling(bool enableMultisampling)
{
  if (enableMultisampling)
  {
    GLCall(glEnable(GL_MULTISAMPLE));
  }
  else
  {
    GLCall(glDisable(GL_MULTISAMPLE));
  }
}

void GLRenderDevice::EnableDepthClip(bool enableDepthClip)
{
  if (enableDepthClip)
  {
    GLCall(glEnable(GL_DEPTH_CLAMP));
  }
  else
  {
    GLCall(glDisable(GL_DEPTH_CLAMP));
  }
}

void GLRenderDevice::EnableAntialiasedLine(bool enableAntialiasedLine)
{
  if (enableAntialiasedLine)
  {
    GLCall(glEnable(GL_LINE_SMOOTH));
  }
  else
  {
    GLCall(glDisable(GL_LINE_SMOOTH));
  }
}

void GLRenderDevice::EnableStencilTest(bool enableStencilTest)
{
  if (enableStencilTest)
  {
    GLCall(glEnable(GL_STENCIL_TEST));
  }
  else
  {
    GLCall(glDisable(GL_STENCIL_TEST));
  }
}

void GLRenderDevice::EnableDepthTest(bool enableDepthTest)
{
  if (enableDepthTest)
  {
    GLCall(glEnable(GL_DEPTH_TEST));
  }
  else
  {
    GLCall(glDisable(GL_DEPTH_TEST));
  }
}

void GLRenderDevice::EnableDepthWrite(bool enableDepthWrite)
{
  if (enableDepthWrite)
  {
    GLCall(glDepthMask(GL_TRUE));
  }
  else
  {
    GLCall(glDepthMask(GL_FALSE));
  }
}

void GLRenderDevice::EnableBlend(bool enableBlend)
{
  if (enableBlend)
  {
    GLCall(glEnable(GL_BLEND));
  }
  else
  {
    GLCall(glDisable(GL_BLEND));
  }
}

#include "Renderer.h"

#include "../Maths/Matrix4.hpp"
#include "../Utility/Assert.hpp"
#include "../Utility/String.hpp"
#include "../RenderApi/GL/GLRenderDevice.hpp"
#include "../RenderApi/PipelineState.hpp"
#include "../RenderApi/RenderTarget.hpp"
#include "../RenderApi/SamplerState.hpp"
#include "../RenderApi/ShaderParams.hpp"
#include "../RenderApi/VertexBuffer.hpp"
#include "../SceneManagement/Camera.hpp"
#include "../SceneManagement/Transform.h"
#include "Material.h"
#include "Renderable.hpp"
#include "StaticMesh.h"

struct FullscreenQuadVertex
{
	Vector2 Position;
	Vector2 TexCoord;

	FullscreenQuadVertex(const Vector2& position, const Vector2& texCoord) :
		Position(position), TexCoord(texCoord)
	{}
};

std::vector<FullscreenQuadVertex> FullscreenQuadVertices
{
	FullscreenQuadVertex(Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f)),
	FullscreenQuadVertex(Vector2(1.0f, -1.0f), Vector2(1.0f, 0.0f)),
	FullscreenQuadVertex(Vector2(-1.0f, 1.0f), Vector2(0.0f, 1.0f)),
	FullscreenQuadVertex(Vector2(1.0f, -1.0f), Vector2(1.0f, 0.0f)),
	FullscreenQuadVertex(Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f)),
	FullscreenQuadVertex(Vector2(-1.0f, 1.0f), Vector2(0.0f, 1.0f))
};

struct FrameBufferData
{
  Matrix4 Proj;
  Matrix4 View;
	DirectionalLightData DirectionalLight;
  Vector3 ViewPosition;
};

struct ObjectBufferData
{
	Matrix4 Model;	
};

std::shared_ptr<RenderDevice> Renderer::_renderDevice;

std::shared_ptr<RenderDevice> Renderer::GetRenderDevice()
{
	Assert::ThrowIfTrue(_renderDevice == nullptr, "Render device has not been initialized");
	return _renderDevice;
}

Renderer::Renderer(const RendererDesc& desc) : _desc(desc)
{
  try
  {
    Assert::ThrowIfFalse(desc.RenderApi == RenderApi::GL41, "Only OpenGL 4.1 is supported");
    
    RenderDeviceDesc renderDeviceDesc;
    renderDeviceDesc.RenderWidth = _desc.RenderWidth;
    renderDeviceDesc.RenderHeight = _desc.RenderHeight;
    _renderDevice.reset(new GLRenderDevice(renderDeviceDesc));
    
    InitPipelineStates();
    InitFrameBuffer();
		InitObjectBuffer();
		InitFullscreenQuad();
		InitLightingPass();
		InitGBufferDebugPass();
  }
  catch (const std::exception& exception)
  {
    throw std::runtime_error(std::string("Could not initialize Renderer\n") + exception.what());
  }
}

void Renderer::PushRenderable(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Transform>& transform)
{
  _renderables.emplace_back(renderable, transform);
}

void Renderer::DrawFrame()
{
  StartFrame();
	GeometryPass();
	switch (_gBufferDisplay)
	{
		default:
		case GBufferDisplayType::Disabled:
			LightingPass();
			break;
		case GBufferDisplayType::Position:
			GBufferDebugPass(0);
			break;
		case GBufferDisplayType::Normal:
			GBufferDebugPass(1);
			break;
		case GBufferDisplayType::Albedo:
			GBufferDebugPass(2);
			break;
	}		
	EndFrame();
}

void Renderer::InitPipelineStates()
{
  ShaderDesc vsDesc;
  vsDesc.EntryPoint = "main";
  vsDesc.ShaderLang = ShaderLang::Glsl;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::LoadFromFile("./../../Resources/Shaders/GeometryPassVS.glsl");
  
  ShaderDesc psDesc;
  psDesc.EntryPoint = "main";
  psDesc.ShaderLang = ShaderLang::Glsl;
  psDesc.ShaderType = ShaderType::Pixel;
  psDesc.Source = String::LoadFromFile("./../../Resources/Shaders/GeometryPassPS.glsl");
  
  std::vector<VertexLayoutDesc> vertexLayoutDesc
  {
    VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
		VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
		VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
		VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
		VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)
  };
  
  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));
  shaderParams->AddParam(ShaderParam("FrameBuffer", ShaderParamType::ConstBuffer, 1));
	shaderParams->AddParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));
  
	RasterizerStateDesc rasterizerStateDesc;
	rasterizerStateDesc.CullMode = CullMode::CounterClockwise;
	rasterizerStateDesc.DepthClipEnabled = true;

  try
  {
    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
    pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
    pipelineDesc.BlendState = _renderDevice->CreateBlendState(BlendStateDesc());
    pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
    pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;
    
    _geomPassPso = _renderDevice->CreatePipelineState(pipelineDesc);
  }
  catch (const std::exception& exception)
  {
    throw std::runtime_error("Unable to initialize pipeline states. " + std::string(exception.what()));
  }

	try
	{
		SamplerStateDesc desc;
    desc.AddressingMode = AddressingMode{ TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap };
		desc.MinFiltering = TextureFilteringMode::Linear;
		desc.MinFiltering = TextureFilteringMode::Linear;
		desc.MipFiltering = TextureFilteringMode::Linear;
		_basicSamplerState = _renderDevice->CreateSamplerState(desc);
	}
	catch (const std::exception& ex)
	{
		throw std::runtime_error("Unable to initalize sampler state. " + std::string(ex.what()));
	}

	try
	{
		TextureDesc colourTexDesc;
		colourTexDesc.Width = GetRenderWidth();
		colourTexDesc.Height = GetRenderHeight();
		colourTexDesc.Usage = TextureUsage::RenderTarget;
		colourTexDesc.Type = TextureType::Texture2D;
		colourTexDesc.Format = TextureFormat::RGB16F;

		TextureDesc depthStencilDesc;
		depthStencilDesc.Width = GetRenderWidth();
		depthStencilDesc.Height = GetRenderHeight();
		depthStencilDesc.Usage = TextureUsage::DepthStencil;
		depthStencilDesc.Type = TextureType::Texture2D;
		depthStencilDesc.Format = TextureFormat::D24S8;

		RenderTargetDesc rtDesc;
		rtDesc.ColourTargets[0] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.ColourTargets[1] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.ColourTargets[2] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.DepthStencilTarget = _renderDevice->CreateTexture(depthStencilDesc);
		rtDesc.Height = GetRenderHeight();
		rtDesc.Width = GetRenderWidth();

		_gBuffer = _renderDevice->CreateRenderTarget(rtDesc);
	}
	catch (const std::exception& ex)
	{
		throw std::runtime_error("Unable to initalize render targets. " + std::string(ex.what()));
	}
}

void Renderer::InitFrameBuffer()
{
  try
  {
    GpuBufferDesc perShaderBuffDesc;
    perShaderBuffDesc.BufferType = BufferType::Constant;
    perShaderBuffDesc.BufferUsage = BufferUsage::Dynamic;
    perShaderBuffDesc.ByteCount = sizeof(FrameBufferData);
    _frameBuffer = _renderDevice->CreateGpuBuffer(perShaderBuffDesc);
  }
  catch (const std::exception& exception)
  {
    throw std::runtime_error(std::string("Could not initialize camera constant buffer\n") + exception.what());
  }
}

void Renderer::InitObjectBuffer()
{
	try
	{
		GpuBufferDesc perShaderBuffDesc;
		perShaderBuffDesc.BufferType = BufferType::Constant;
		perShaderBuffDesc.BufferUsage = BufferUsage::Dynamic;
		perShaderBuffDesc.ByteCount = sizeof(ObjectBufferData);
		_objectBuffer = _renderDevice->CreateGpuBuffer(perShaderBuffDesc);
	}
	catch (const std::exception& exception)
	{
		throw std::runtime_error(std::string("Could not initialize object constant buffer\n") + exception.what());
	}
}

void Renderer::InitLightingPass()
{
	ShaderDesc vsDesc;
	vsDesc.EntryPoint = "main";
	vsDesc.ShaderLang = ShaderLang::Glsl;
	vsDesc.ShaderType = ShaderType::Vertex;
	vsDesc.Source = String::LoadFromFile("./../../Resources/Shaders/FSPassThroughVS.glsl");

	ShaderDesc psDesc;
	psDesc.EntryPoint = "main";
	psDesc.ShaderLang = ShaderLang::Glsl;
	psDesc.ShaderType = ShaderType::Pixel;
	psDesc.Source = String::LoadFromFile("./../../Resources/Shaders/DeferredLightingPassPS.glsl");

	std::vector<VertexLayoutDesc> vertexLayoutDesc
	{
		VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
		VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
	};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("PositionMap", ShaderParamType::Texture, 0));
	shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
	shaderParams->AddParam(ShaderParam("AlbedoSpecMap", ShaderParamType::Texture, 2));
	shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));
	shaderParams->AddParam(ShaderParam("FrameBuffer", ShaderParamType::ConstBuffer, 1));

	RasterizerStateDesc rasterizerStateDesc;
	rasterizerStateDesc.CullMode = CullMode::None;

	try
	{
		PipelineStateDesc pipelineDesc;
		pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
		pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
		pipelineDesc.BlendState = _renderDevice->CreateBlendState(BlendStateDesc());
		pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
		pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
		pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
		pipelineDesc.ShaderParams = shaderParams;

		_lightPassPso = _renderDevice->CreatePipelineState(pipelineDesc);
	}
	catch (const std::exception& exception)
	{
		throw std::runtime_error("Unable to initialize pipeline states. " + std::string(exception.what()));
	}
}

void Renderer::InitFullscreenQuad()
{
	if (!_fsQuadBuffer)
	{
		VertexBufferDesc vtxBuffDesc;
		vtxBuffDesc.BufferUsage = BufferUsage::Default;
		vtxBuffDesc.VertexCount = FullscreenQuadVertices.size();
		vtxBuffDesc.VertexSizeBytes = sizeof(FullscreenQuadVertex);
		try
		{
			_fsQuadBuffer = _renderDevice->CreateVertexBuffer(vtxBuffDesc);
			_fsQuadBuffer->WriteData(0, sizeof(FullscreenQuadVertex) * FullscreenQuadVertices.size(), FullscreenQuadVertices.data(), AccessType::WriteOnlyDiscardRange);
		}
		catch (const std::exception& exception)
		{
			throw std::runtime_error("Unable to initialize fullscreen Quad vertex buffer. " + std::string(exception.what()));
		}
	}

	if (!_noMipSamplerState)
	{
		SamplerStateDesc desc;
		desc.AddressingMode = AddressingMode{ TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap };
		desc.MinFiltering = TextureFilteringMode::None;
		desc.MinFiltering = TextureFilteringMode::None;
		desc.MipFiltering = TextureFilteringMode::None;
		try
		{
			_noMipSamplerState = _renderDevice->CreateSamplerState(desc);
		}
		catch (const std::exception& exception)
		{
			throw std::runtime_error("Unable to initialize fullscreen Quad sampler state. " + std::string(exception.what()));
		}
	}
}

void Renderer::InitGBufferDebugPass()
{
	ShaderDesc vsDesc;
	vsDesc.EntryPoint = "main";
	vsDesc.ShaderLang = ShaderLang::Glsl;
	vsDesc.ShaderType = ShaderType::Vertex;
	vsDesc.Source = String::LoadFromFile("./../../Resources/Shaders/FSPassThroughVS.glsl");

	ShaderDesc psDesc;
	psDesc.EntryPoint = "main";
	psDesc.ShaderLang = ShaderLang::Glsl;
	psDesc.ShaderType = ShaderType::Pixel;
	psDesc.Source = String::LoadFromFile("./../../Resources/Shaders/FullscreenQuad.glsl");

	std::vector<VertexLayoutDesc> vertexLayoutDesc
	{
		VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
		VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
	};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("QuadTexture", ShaderParamType::Texture, 0));

	RasterizerStateDesc rasterizerStateDesc;
	rasterizerStateDesc.CullMode = CullMode::None;

	try
	{
		PipelineStateDesc pipelineDesc;
		pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
		pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
		pipelineDesc.BlendState = _renderDevice->CreateBlendState(BlendStateDesc());
		pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
		pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
		pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
		pipelineDesc.ShaderParams = shaderParams;

		_gBufferDebugPso = _renderDevice->CreatePipelineState(pipelineDesc);
	}
	catch (const std::exception& exception)
	{
		throw std::runtime_error("Unable to initialize pipeline states. " + std::string(exception.what()));
	}
}

void Renderer::StartFrame()
{
  FrameBufferData framData;
  framData.Proj = _activeCamera->GetProjection();
	framData.DirectionalLight = _directionalLight;
  framData.View = _activeCamera->GetView();
  framData.ViewPosition = _activeCamera->GetPosition();
  _frameBuffer->WriteData(0, sizeof(FrameBufferData), &framData);
	  
	_renderDevice->SetConstantBuffer(0, _objectBuffer);
	_renderDevice->SetConstantBuffer(1, _frameBuffer);
	_renderDevice->SetSamplerState(0, _basicSamplerState);
	_renderDevice->ClearBuffers(RTT_Colour | RTT_Depth | RTT_Stencil);
}

void Renderer::EndFrame()
{
}

void Renderer::GeometryPass()
{
	_renderDevice->SetPipelineState(_geomPassPso);
	_renderDevice->SetRenderTarget(_gBuffer);
	_renderDevice->ClearBuffers(RTT_Colour | RTT_Depth | RTT_Stencil);
  
  _renderDevice->SetSamplerState(0, _basicSamplerState);
	for (auto& renderable : _renderables)
	{
		for (uint32 i = 0; i < renderable.Renderable->GetMeshCount(); i++)
		{
			auto mesh = renderable.Renderable->GetMeshAtIndex(i);
			_renderDevice->SetTexture(0, mesh->GetMaterial()->GetTexture("DiffuseMap"));
			_renderDevice->SetVertexBuffer(0, mesh->GetVertexData());

			ObjectBufferData objData;
			objData.Model = renderable.Transform->Get();
			_objectBuffer->WriteData(0, sizeof(ObjectBufferData), &objData);

			if (mesh->IsIndexed())
			{
				_renderDevice->SetIndexBuffer(mesh->GetIndexData());
				_renderDevice->DrawIndexed(mesh->GetIndexCount(), 0, 0);
			}
			else
			{
				_renderDevice->Draw(mesh->GetVertexCount(), 0);
			}
		}
	}
	_renderables.clear();
}

void Renderer::LightingPass()
{
	_renderDevice->SetRenderTarget(nullptr);
	_renderDevice->SetPipelineState(_lightPassPso);
	_renderDevice->SetTexture(0, _gBuffer->GetColourTarget(0));
	_renderDevice->SetTexture(1, _gBuffer->GetColourTarget(1));
	_renderDevice->SetTexture(2, _gBuffer->GetColourTarget(2));
	_renderDevice->SetSamplerState(0, _noMipSamplerState);
	_renderDevice->SetSamplerState(1, _noMipSamplerState);
	_renderDevice->SetSamplerState(2, _noMipSamplerState);
	_renderDevice->SetVertexBuffer(0, _fsQuadBuffer);
  _renderDevice->SetConstantBuffer(0, _objectBuffer);
  _renderDevice->SetConstantBuffer(1, _frameBuffer);
	_renderDevice->Draw(6, 0);
}

void Renderer::GBufferDebugPass(uint32 i)
{
	_renderDevice->SetRenderTarget(nullptr);
	_renderDevice->SetPipelineState(_gBufferDebugPso);
	_renderDevice->SetTexture(0, _gBuffer->GetColourTarget(i));
	_renderDevice->SetSamplerState(0, _noMipSamplerState);
	_renderDevice->SetVertexBuffer(0, _fsQuadBuffer);
	_renderDevice->Draw(6, 0);
}

#include "Renderer.h"

#include <chrono>
#include <random>
#include <stdexcept>

#include "../Geometry/MeshFactory.h"
#include "../Maths/AABB.hpp"
#include "../Utility/Assert.hpp"
#include "../Utility/String.hpp"
#include "../RenderApi/GL/GLRenderDevice.hpp"
#include "../RenderApi/PipelineState.hpp"
#include "../RenderApi/RenderTarget.hpp"
#include "../RenderApi/SamplerState.hpp"
#include "../RenderApi/ShaderParams.hpp"
#include "../RenderApi/VertexBuffer.hpp"
#include "../SceneManagement/Transform.h"
#include "Material.hpp"
#include "Renderable.hpp"
#include "RenderQueue.hpp"
#include "StaticMesh.h"

struct FullscreenQuadVertex
{
	Vector2 Position;
	Vector2 TexCoord;

	FullscreenQuadVertex(const Vector2 &position, const Vector2 &texCoord) : Position(position), TexCoord(texCoord)
	{
	}
};

std::vector<FullscreenQuadVertex> FullscreenQuadVertices{
		FullscreenQuadVertex(Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f)),
		FullscreenQuadVertex(Vector2(1.0f, -1.0f), Vector2(1.0f, 0.0f)),
		FullscreenQuadVertex(Vector2(-1.0f, 1.0f), Vector2(0.0f, 1.0f)),
		FullscreenQuadVertex(Vector2(1.0f, -1.0f), Vector2(1.0f, 0.0f)),
		FullscreenQuadVertex(Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f)),
		FullscreenQuadVertex(Vector2(-1.0f, 1.0f), Vector2(0.0f, 1.0f))};

struct MaterialBufferData
{
	struct TextureMapFlagData
	{
		int32 Diffuse = 0;
		int32 Normal = 0;
		int32 Specular = 0;
		int32 Opacity = 0;
	} EnabledTextureMaps;
	Colour Ambient = Colour::White;
	Colour Diffuse = Colour::White;
	Colour Specular = Colour::White;
	float32 Exponent = 1.0f;
	int32 GreyscaleSpecular = 1;
};

std::shared_ptr<RenderDevice> Renderer::_renderDevice;

std::shared_ptr<RenderDevice> Renderer::GetRenderDevice()
{
	ASSERT_FALSE(_renderDevice == nullptr, "Render device has not been initialized");
	return _renderDevice;
}

Renderer::Renderer(const RendererDesc &desc) : _desc(desc),
																							 _opaqueQueue(new RenderQueue),
																							 _ssaoEnabled(true),
																							 _hdrEnabled(true),
																							 _shadowResolution(2048),
																							 _shadowOrthographicSize(10.0f)
{
	try
	{
		ASSERT_TRUE(desc.RenderApi == RenderApi::GL41, "Only OpenGL 4.1 is supported");

		RenderDeviceDesc renderDeviceDesc;
		renderDeviceDesc.RenderWidth = _desc.RenderWidth;
		renderDeviceDesc.RenderHeight = _desc.RenderHeight;
		_renderDevice.reset(new GLRenderDevice(renderDeviceDesc));

		GenerateSsaoKernel();

		InitDepthBuffer();
		InitShadowDepthPass();
		InitDepthRenderTarget();
		InitFrameBuffer();
		InitMaterialBuffer();
		InitFullscreenQuad();
		InitPointLightMesh();
		InitDepthDebugPass();
		InitGBufferDebugPass();
		InitSsaoPass();
		InitSsaoBlurPass();

		InitPointLightBuffer();
		InitPointLightConstantsBuffer();

		InitGBufferPass();
		InitPointLightPass();
		InitMergePass();
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error(std::string("Could not initialize Renderer\n") + exception.what());
	}
}

void Renderer::Push(const std::shared_ptr<Renderable> &renderable, const std::shared_ptr<Transform> &transform, const Aabb &bounds)
{
}

void Renderer::BindCamera(std::shared_ptr<CameraNode> camera)
{
	_camera = camera;
}

void Renderer::SubmitLight(const sptr<LightNode> &lightNode)
{
	// TODO Multiple light sources and types

	if (lightNode->GetLightType() == LightType::Directional)
	{
		const Quaternion orientation = lightNode->GetTransform().GetRotation();
		_directionalLightData.Colour = lightNode->GetColour();
		_directionalLightData.Direction = orientation.Rotate(Vector3::Identity);
	}
	else if (lightNode->GetLightType() == LightType::Point)
	{
		_pointLights.push_back(lightNode);
	}
}

void Renderer::SubmitRenderable(const sptr<Renderable> &renderable)
{
	// TODO Implement RenderQueue

	_renderables.push_back(renderable);
}

void Renderer::DrawFrame()
{
	auto frameStart = std::chrono::high_resolution_clock::now();

	StartFrame();

	GBufferFillPass();

	switch (_debugDisplayType)
	{
	default:
	case DebugDisplayType::Disabled:
		PointLightPass();
		MergePass();
		break;
	case DebugDisplayType::Diffuse:
		RTOTextureDebugPass(_gBufferRto->GetColourTarget(0));
		break;
	case DebugDisplayType::Normal:
		RTOTextureDebugPass(_gBufferRto->GetColourTarget(1));
		break;
	case DebugDisplayType::Depth:
		DepthDebugPass(_gBufferRto->GetDepthStencilTarget());
		break;
	case DebugDisplayType::Emissive:
		PointLightPass();
		RTOTextureDebugPass(_pointLightRto->GetColourTarget(0));
		break;
	case DebugDisplayType::Specular:
		PointLightPass();
		RTOTextureDebugPass(_pointLightRto->GetColourTarget(1));
		break;
	}
	EndFrame();

	_renderables.clear();
	_opaqueQueue->Clear();
	_pointLights.clear();

	auto frameEnd = std::chrono::high_resolution_clock::now();
	_renderTimings.Frame = std::chrono::duration_cast<std::chrono::nanoseconds>(frameEnd - frameStart).count();
}

float32 Renderer::GetOrthographicSize() const
{
	return _shadowOrthographicSize;
}

void Renderer::SetOrthographicSize(float32 shadowOrthographicSize)
{
	_shadowOrthographicSize = shadowOrthographicSize;
}

void Renderer::InitShadowDepthPass()
{
	ShaderDesc vsDesc;
	vsDesc.EntryPoint = "main";
	vsDesc.ShaderLang = ShaderLang::Glsl;
	vsDesc.ShaderType = ShaderType::Vertex;
	vsDesc.Source = String::LoadFromFile("./Shaders/ShadowDepth.vert");

	ShaderDesc psDesc;
	psDesc.EntryPoint = "main";
	psDesc.ShaderLang = ShaderLang::Glsl;
	psDesc.ShaderType = ShaderType::Pixel;
	psDesc.Source = String::LoadFromFile("./Shaders/ShadowDepth.frag");

	std::vector<VertexLayoutDesc> vertexLayoutDesc{
			VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
			VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));
	shaderParams->AddParam(ShaderParam("ShadowBuffer", ShaderParamType::ConstBuffer, 3));

	try
	{
		PipelineStateDesc pipelineDesc;
		pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
		pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
		pipelineDesc.BlendState = _renderDevice->CreateBlendState(BlendStateDesc());
		pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(RasterizerStateDesc());
		pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
		pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
		pipelineDesc.ShaderParams = shaderParams;

		_shadowDepthPso = _renderDevice->CreatePipelineState(pipelineDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize shadow-depth pipeline states. " + std::string(exception.what()));
	}
}

void Renderer::InitDepthRenderTarget()
{
	try
	{
		TextureDesc depthStencilDesc;
		depthStencilDesc.Width = _desc.ShadowRes.X;
		depthStencilDesc.Height = _desc.ShadowRes.Y;
		depthStencilDesc.Usage = TextureUsage::DepthStencil;
		depthStencilDesc.Type = TextureType::Texture2D;
		depthStencilDesc.Format = TextureFormat::D24S8;

		RenderTargetDesc rtDesc;
		rtDesc.DepthStencilTarget = _renderDevice->CreateTexture(depthStencilDesc);
		rtDesc.Height = _desc.ShadowRes.Y;
		rtDesc.Width = _desc.ShadowRes.X;

		_depthRenderTarget = _renderDevice->CreateRenderTarget(rtDesc);
	}
	catch (const std::exception &ex)
	{
		throw std::runtime_error("Unable to initalize shadow depth render targets. " + std::string(ex.what()));
	}
}

void Renderer::InitDepthBuffer()
{
	try
	{
		GpuBufferDesc desc;
		desc.BufferType = BufferType::Constant;
		desc.BufferUsage = BufferUsage::Stream;
		desc.ByteCount = sizeof(FrameBufferData);
		_depthBuffer = _renderDevice->CreateGpuBuffer(desc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error(std::string("Could not initialize depth constant buffer\n") + exception.what());
	}
}

void Renderer::InitGBufferPass()
{
	ShaderDesc vsDesc;
	vsDesc.EntryPoint = "main";
	vsDesc.ShaderLang = ShaderLang::Glsl;
	vsDesc.ShaderType = ShaderType::Vertex;
	vsDesc.Source = String::LoadFromFile("./Shaders/Gbuffer.vert");

	ShaderDesc psDesc;
	psDesc.EntryPoint = "main";
	psDesc.ShaderLang = ShaderLang::Glsl;
	psDesc.ShaderType = ShaderType::Pixel;
	psDesc.Source = String::LoadFromFile("./Shaders/Gbuffer.frag");

	std::vector<VertexLayoutDesc> vertexLayoutDesc{
			VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
			VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));
	shaderParams->AddParam(ShaderParam("MaterialBuffer", ShaderParamType::ConstBuffer, 2));
	shaderParams->AddParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));
	shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
	shaderParams->AddParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 2));
	shaderParams->AddParam(ShaderParam("OpacityMap", ShaderParamType::Texture, 3));

	RasterizerStateDesc rasterizerStateDesc;
	rasterizerStateDesc.CullMode = CullMode::CounterClockwise;

	BlendStateDesc blendStateDesc{};
	blendStateDesc.RTBlendState[0].BlendEnabled = true;
	blendStateDesc.RTBlendState[0].BlendAlpha = BlendDesc(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha, BlendOperation::Add);

	try
	{
		PipelineStateDesc pipelineDesc;
		pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
		pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
		pipelineDesc.BlendState = _renderDevice->CreateBlendState(blendStateDesc);
		pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
		pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
		pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
		pipelineDesc.ShaderParams = shaderParams;

		_gBufferPso = _renderDevice->CreatePipelineState(pipelineDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize pipeline states. " + std::string(exception.what()));
	}

	try
	{
		SamplerStateDesc desc;
		desc.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
		desc.MinFiltering = TextureFilteringMode::Linear;
		desc.MinFiltering = TextureFilteringMode::Linear;
		desc.MipFiltering = TextureFilteringMode::Linear;
		_basicSamplerState = _renderDevice->CreateSamplerState(desc);
	}
	catch (const std::exception &ex)
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
		colourTexDesc.Format = TextureFormat::RGBA8;

		TextureDesc depthStencilDesc;
		depthStencilDesc.Width = GetRenderWidth();
		depthStencilDesc.Height = GetRenderHeight();
		depthStencilDesc.Usage = TextureUsage::Depth;
		depthStencilDesc.Type = TextureType::Texture2D;
		depthStencilDesc.Format = TextureFormat::D24;

		RenderTargetDesc rtDesc;
		rtDesc.ColourTargets[0] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.ColourTargets[1] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.ColourTargets[2] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.DepthStencilTarget = _renderDevice->CreateTexture(depthStencilDesc);
		rtDesc.Height = GetRenderHeight();
		rtDesc.Width = GetRenderWidth();

		_gBufferRto = _renderDevice->CreateRenderTarget(rtDesc);
	}
	catch (const std::exception &ex)
	{
		throw std::runtime_error("Unable to initalize render targets. " + std::string(ex.what()));
	}
}

void Renderer::InitPointLightPass()
{
	ShaderDesc vsDesc;
	vsDesc.EntryPoint = "main";
	vsDesc.ShaderLang = ShaderLang::Glsl;
	vsDesc.ShaderType = ShaderType::Vertex;
	vsDesc.Source = String::LoadFromFile("./Shaders/PointLights.vert");

	ShaderDesc psDesc;
	psDesc.EntryPoint = "main";
	psDesc.ShaderLang = ShaderLang::Glsl;
	psDesc.ShaderType = ShaderType::Pixel;
	psDesc.Source = String::LoadFromFile("./Shaders/PointLights.frag");

	std::vector<VertexLayoutDesc> vertexLayoutDesc{
			VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
			VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
			VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));
	shaderParams->AddParam(ShaderParam("PointLightPassConstants", ShaderParamType::ConstBuffer, 1));
	shaderParams->AddParam(ShaderParam("PointLightBuffer", ShaderParamType::ConstBuffer, 2));
	shaderParams->AddParam(ShaderParam("DepthMap", ShaderParamType::Texture, 0));
	shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
	shaderParams->AddParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 2));

	RasterizerStateDesc rasterizerStateDesc{};
	rasterizerStateDesc.CullMode = CullMode::Clockwise;

	DepthStencilStateDesc depthStencilStateDesc{};
	depthStencilStateDesc.DepthReadEnabled = false;
	depthStencilStateDesc.DepthWriteEnabled = false;

	BlendStateDesc blendStateDesc{};
	blendStateDesc.RTBlendState[0].BlendEnabled = true;
	blendStateDesc.RTBlendState[0].Blend = BlendDesc(BlendFactor::One, BlendFactor::One, BlendOperation::Add);

	try
	{
		PipelineStateDesc pipelineDesc;
		pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
		pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
		pipelineDesc.BlendState = _renderDevice->CreateBlendState(blendStateDesc);
		pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
		pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(depthStencilStateDesc);
		pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
		pipelineDesc.ShaderParams = shaderParams;

		_pointLightCWPso = _renderDevice->CreatePipelineState(pipelineDesc);

		rasterizerStateDesc.CullMode = CullMode::CounterClockwise;
		pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
		_pointLightCCWPso = _renderDevice->CreatePipelineState(pipelineDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize pipeline states. " + std::string(exception.what()));
	}

	try
	{
		TextureDesc colourTexDesc;
		colourTexDesc.Width = GetRenderWidth();
		colourTexDesc.Height = GetRenderHeight();
		colourTexDesc.Usage = TextureUsage::RenderTarget;
		colourTexDesc.Type = TextureType::Texture2D;
		colourTexDesc.Format = TextureFormat::RGB8;

		RenderTargetDesc rtDesc;
		rtDesc.ColourTargets[0] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.ColourTargets[1] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.Height = GetRenderHeight();
		rtDesc.Width = GetRenderWidth();

		_pointLightRto = _renderDevice->CreateRenderTarget(rtDesc);
	}
	catch (const std::exception &ex)
	{
		throw std::runtime_error("Unable to initalize render targets. " + std::string(ex.what()));
	}
}

void Renderer::InitMergePass()
{
	ShaderDesc vsDesc;
	vsDesc.EntryPoint = "main";
	vsDesc.ShaderLang = ShaderLang::Glsl;
	vsDesc.ShaderType = ShaderType::Vertex;
	vsDesc.Source = String::LoadFromFile("./Shaders/FSPassThrough.vert");

	ShaderDesc psDesc;
	psDesc.EntryPoint = "main";
	psDesc.ShaderLang = ShaderLang::Glsl;
	psDesc.ShaderType = ShaderType::Pixel;
	psDesc.Source = String::LoadFromFile("./Shaders/Merge.frag");

	std::vector<VertexLayoutDesc> vertexLayoutDesc{
			VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
			VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
	};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));
	shaderParams->AddParam(ShaderParam("EmissiveMap", ShaderParamType::Texture, 1));
	shaderParams->AddParam(ShaderParam("SpecularMap", ShaderParamType::Texture, 2));

	RasterizerStateDesc rasterizerStateDesc;
	rasterizerStateDesc.CullMode = CullMode::None;

	PipelineStateDesc pipelineDesc;
	pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
	pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
	pipelineDesc.BlendState = _renderDevice->CreateBlendState(BlendStateDesc());
	pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
	pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
	pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
	pipelineDesc.ShaderParams = shaderParams;

	_mergePso = _renderDevice->CreatePipelineState(pipelineDesc);
}

void Renderer::InitPointLightConstantsBuffer()
{
	try
	{
		GpuBufferDesc perShaderBuffDesc;
		perShaderBuffDesc.BufferType = BufferType::Constant;
		perShaderBuffDesc.BufferUsage = BufferUsage::Stream;
		perShaderBuffDesc.ByteCount = sizeof(PointLightConstantsBuffer);
		_pointLightConstantsBuffer = _renderDevice->CreateGpuBuffer(perShaderBuffDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error(std::string("Could not initialize point light constant buffer\n") + exception.what());
	}
}

void Renderer::InitPointLightBuffer()
{
	try
	{
		GpuBufferDesc perShaderBuffDesc;
		perShaderBuffDesc.BufferType = BufferType::Constant;
		perShaderBuffDesc.BufferUsage = BufferUsage::Stream;
		perShaderBuffDesc.ByteCount = sizeof(PointLightBuffer);
		_pointLightBuffer = _renderDevice->CreateGpuBuffer(perShaderBuffDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error(std::string("Could not initialize point light buffer\n") + exception.what());
	}
}

void Renderer::InitFrameBuffer()
{
	try
	{
		GpuBufferDesc perShaderBuffDesc;
		perShaderBuffDesc.BufferType = BufferType::Constant;
		perShaderBuffDesc.BufferUsage = BufferUsage::Stream;
		perShaderBuffDesc.ByteCount = sizeof(FrameBufferData);
		_frameBuffer = _renderDevice->CreateGpuBuffer(perShaderBuffDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error(std::string("Could not initialize camera constant buffer\n") + exception.what());
	}
}

void Renderer::InitMaterialBuffer()
{
	try
	{
		GpuBufferDesc perShaderBuffDesc;
		perShaderBuffDesc.BufferType = BufferType::Constant;
		perShaderBuffDesc.BufferUsage = BufferUsage::Stream;
		perShaderBuffDesc.ByteCount = sizeof(MaterialBufferData);
		_materialBuffer = _renderDevice->CreateGpuBuffer(perShaderBuffDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error(std::string("Could not initialize material constant buffer\n") + exception.what());
	}
}

void Renderer::InitSsaoPass()
{
	try
	{
		TextureDesc noiseTextureDesc;
		noiseTextureDesc.Format = TextureFormat::RGB8;
		noiseTextureDesc.Height = 4;
		noiseTextureDesc.Width = 4;
		noiseTextureDesc.Type = TextureType::Texture2D;
		_ssaoNoiseTexture = _renderDevice->CreateTexture(noiseTextureDesc);

		std::shared_ptr<ImageData> pixelData(new ImageData(4, 4, 1, ImageFormat::RGB8));
		pixelData->WriteData(reinterpret_cast<ubyte *>(_ssaoNoise.data()));
		_ssaoNoiseTexture->WriteData(0, 0, pixelData);

		SamplerStateDesc noiseSamplerStateDesc;
		noiseSamplerStateDesc.AddressingMode.U = TextureAddressMode::Wrap;
		noiseSamplerStateDesc.AddressingMode.V = TextureAddressMode::Wrap;
		noiseSamplerStateDesc.AddressingMode.W = TextureAddressMode::Wrap;
		_ssaoSamplerState = _renderDevice->CreateSamplerState(noiseSamplerStateDesc);

		ShaderDesc vsDesc;
		vsDesc.EntryPoint = "main";
		vsDesc.ShaderLang = ShaderLang::Glsl;
		vsDesc.ShaderType = ShaderType::Vertex;
		vsDesc.Source = String::LoadFromFile("./Shaders/FSPassThrough.vert");

		ShaderDesc psDesc;
		psDesc.EntryPoint = "main";
		psDesc.ShaderLang = ShaderLang::Glsl;
		psDesc.ShaderType = ShaderType::Pixel;
		psDesc.Source = String::LoadFromFile("./Shaders/Ssao.frag");

		std::vector<VertexLayoutDesc> vertexLayoutDesc{
				VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
				VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
		};

		std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
		shaderParams->AddParam(ShaderParam("PositionMap", ShaderParamType::Texture, 0));
		shaderParams->AddParam(ShaderParam("NormalMap", ShaderParamType::Texture, 1));
		shaderParams->AddParam(ShaderParam("NoiseMap", ShaderParamType::Texture, 2));
		shaderParams->AddParam(ShaderParam("FrameBuffer", ShaderParamType::ConstBuffer, 1));
		shaderParams->AddParam(ShaderParam("SsaoBuffer", ShaderParamType::ConstBuffer, 4));

		RasterizerStateDesc rasterizerStateDesc;
		rasterizerStateDesc.CullMode = CullMode::None;

		PipelineStateDesc pipelineDesc;
		pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
		pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
		pipelineDesc.BlendState = _renderDevice->CreateBlendState(BlendStateDesc());
		pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
		pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
		pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
		pipelineDesc.ShaderParams = shaderParams;

		_ssaoPassPso = _renderDevice->CreatePipelineState(pipelineDesc);

		_ssaoDetailsData.QuadHeight = GetRenderHeight();
		_ssaoDetailsData.QuadWidth = GetRenderWidth();
		for (uint32 i = 0; i < MaxKernelSize; i++)
		{
			_ssaoDetailsData.Samples[i] = Vector4(_ssaoKernel[i], 0.0f);
		}
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize SSAO pass. " + std::string(exception.what()));
	}

	try
	{
		TextureDesc colourTexDesc;
		colourTexDesc.Width = GetRenderWidth();
		colourTexDesc.Height = GetRenderHeight();
		colourTexDesc.Usage = TextureUsage::RenderTarget;
		colourTexDesc.Type = TextureType::Texture2D;
		colourTexDesc.Format = TextureFormat::R8;

		RenderTargetDesc rtDesc;
		rtDesc.ColourTargets[0] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.Height = GetRenderHeight();
		rtDesc.Width = GetRenderWidth();

		_ssaoRT = _renderDevice->CreateRenderTarget(rtDesc);
	}
	catch (const std::exception &ex)
	{
		throw std::runtime_error("Unable to initalize SSAO render target. " + std::string(ex.what()));
	}
}

void Renderer::InitSsaoBlurPass()
{
	try
	{
		ShaderDesc vsDesc;
		vsDesc.EntryPoint = "main";
		vsDesc.ShaderLang = ShaderLang::Glsl;
		vsDesc.ShaderType = ShaderType::Vertex;
		vsDesc.Source = String::LoadFromFile("./Shaders/FSPassThrough.vert");

		ShaderDesc psDesc;
		psDesc.EntryPoint = "main";
		psDesc.ShaderLang = ShaderLang::Glsl;
		psDesc.ShaderType = ShaderType::Pixel;
		psDesc.Source = String::LoadFromFile("./Shaders/SsaoBlur.frag");

		std::vector<VertexLayoutDesc> vertexLayoutDesc{
				VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
				VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
		};

		std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
		shaderParams->AddParam(ShaderParam("SsaoMap", ShaderParamType::Texture, 0));

		RasterizerStateDesc rasterizerStateDesc;
		rasterizerStateDesc.CullMode = CullMode::None;

		PipelineStateDesc pipelineDesc;
		pipelineDesc.VS = _renderDevice->CreateShader(vsDesc);
		pipelineDesc.PS = _renderDevice->CreateShader(psDesc);
		pipelineDesc.BlendState = _renderDevice->CreateBlendState(BlendStateDesc());
		pipelineDesc.RasterizerState = _renderDevice->CreateRasterizerState(rasterizerStateDesc);
		pipelineDesc.DepthStencilState = _renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
		pipelineDesc.VertexLayout = _renderDevice->CreateVertexLayout(vertexLayoutDesc);
		pipelineDesc.ShaderParams = shaderParams;

		_ssaoBlurPassPso = _renderDevice->CreatePipelineState(pipelineDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize SSAO blur pass. " + std::string(exception.what()));
	}

	try
	{
		TextureDesc colourTexDesc;
		colourTexDesc.Width = GetRenderWidth();
		colourTexDesc.Height = GetRenderHeight();
		colourTexDesc.Usage = TextureUsage::RenderTarget;
		colourTexDesc.Type = TextureType::Texture2D;
		colourTexDesc.Format = TextureFormat::R8;

		RenderTargetDesc rtDesc;
		rtDesc.ColourTargets[0] = _renderDevice->CreateTexture(colourTexDesc);
		rtDesc.Height = GetRenderHeight();
		rtDesc.Width = GetRenderWidth();

		_ssaoBlurRT = _renderDevice->CreateRenderTarget(rtDesc);
	}
	catch (const std::exception &ex)
	{
		throw std::runtime_error("Unable to initalize SSAO render target. " + std::string(ex.what()));
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
		catch (const std::exception &exception)
		{
			throw std::runtime_error("Unable to initialize fullscreen Quad vertex buffer. " + std::string(exception.what()));
		}
	}

	if (!_noMipSamplerState)
	{
		SamplerStateDesc desc;
		desc.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
		desc.MinFiltering = TextureFilteringMode::None;
		desc.MinFiltering = TextureFilteringMode::None;
		desc.MipFiltering = TextureFilteringMode::None;
		try
		{
			_noMipSamplerState = _renderDevice->CreateSamplerState(desc);
		}
		catch (const std::exception &exception)
		{
			throw std::runtime_error("Unable to initialize fullscreen Quad sampler state. " + std::string(exception.what()));
		}
	}
}

void Renderer::InitPointLightMesh()
{
	if (_pointLightMesh)
	{
		return;
	}

	_pointLightMesh = MeshFactory::CreateUvSphere();

	GpuBufferDesc desc;
	desc.BufferType = BufferType::Constant;
	desc.BufferUsage = BufferUsage::Stream;
	desc.ByteCount = sizeof(PerObjectBufferData);
	_lightObjectBuffer = Renderer::GetRenderDevice()->CreateGpuBuffer(desc);
}

void Renderer::InitDepthDebugPass()
{
	ShaderDesc vsDesc;
	vsDesc.EntryPoint = "main";
	vsDesc.ShaderLang = ShaderLang::Glsl;
	vsDesc.ShaderType = ShaderType::Vertex;
	vsDesc.Source = String::LoadFromFile("./Shaders/FSPassThrough.vert");

	ShaderDesc psDesc;
	psDesc.EntryPoint = "main";
	psDesc.ShaderLang = ShaderLang::Glsl;
	psDesc.ShaderType = ShaderType::Pixel;
	psDesc.Source = String::LoadFromFile("./Shaders/DepthDebug.frag");

	std::vector<VertexLayoutDesc> vertexLayoutDesc{
			VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
			VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
	};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("QuadTexture", ShaderParamType::Texture, 0));
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

		_depthDebugPso = _renderDevice->CreatePipelineState(pipelineDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize depth debug pipeline states. " + std::string(exception.what()));
	}
}

void Renderer::InitGBufferDebugPass()
{
	ShaderDesc vsDesc;
	vsDesc.EntryPoint = "main";
	vsDesc.ShaderLang = ShaderLang::Glsl;
	vsDesc.ShaderType = ShaderType::Vertex;
	vsDesc.Source = String::LoadFromFile("./Shaders/FSPassThrough.vert");

	ShaderDesc psDesc;
	psDesc.EntryPoint = "main";
	psDesc.ShaderLang = ShaderLang::Glsl;
	psDesc.ShaderType = ShaderType::Pixel;
	psDesc.Source = String::LoadFromFile("./Shaders/FullscreenQuad.frag");

	std::vector<VertexLayoutDesc> vertexLayoutDesc{
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
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize pipeline states. " + std::string(exception.what()));
	}
}

void Renderer::GenerateSsaoKernel()
{
	auto lerp = [](float32 a, float32 b, float32 f)
	{
		return a + f * (b - a);
	};

	std::uniform_real_distribution<float> randomFloat(0.0, 1.0);
	std::default_random_engine generator;

	for (uint32 i = 0; i < 64; i++)
	{
		Vector3 sample(randomFloat(generator) * 2.0f - 1.0f,
									 randomFloat(generator) * 2.0f - 1.0f,
									 randomFloat(generator));
		sample.Normalize();
		sample *= randomFloat(generator);
		float32 scale = 1.0f / 64.0f;

		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		_ssaoKernel.push_back(sample);
	}

	for (uint32 i = 0; i < 16; i++)
	{
		_ssaoNoise.push_back(Vector3(randomFloat(generator) * 2.0f - 1.0f,
																 randomFloat(generator) * 2.0f - 1.0f,
																 0.0f));
	}
}

void Renderer::StartFrame()
{
	_ambientLightData.SsaoEnabled = _ssaoEnabled ? 1 : 0;

	_ssaoDetailsData.Radius = 1.0f;
	_ssaoDetailsData.KernelSize = 16;
	_ssaoDetailsData.Bias = 0.01f;

	_hdrData.Enabled = _hdrEnabled ? 1 : 0;

	FrameBufferData framData;
	framData.Proj = _camera->GetProj();
	framData.ProjViewInvs = (_camera->GetProj() * _camera->GetView()).Inverse();
	framData.DirectionalLight = _directionalLightData;
	framData.AmbientLight = _ambientLightData;
	framData.View = _camera->GetView();
	framData.ViewInvs = framData.View.Inverse();
	framData.ViewPosition = Vector4(_camera->GetTransform().GetPosition(), 1.0f);
	framData.SsaoDetails = _ssaoDetailsData;
	framData.Hdr = _hdrData;
	framData.nearClip = _camera->GetNear();
	framData.farClip = _camera->GetFar();
	_frameBuffer->WriteData(0, sizeof(FrameBufferData), &framData, AccessType::WriteOnlyDiscard);

	_renderDevice->ClearBuffers(RTT_Colour | RTT_Depth | RTT_Stencil);

	_renderCounts.DrawCount = 0;
	_renderCounts.MaterialCount = 0;
	_renderCounts.TriangleCount = 0;

	_activeMaterial = nullptr;

	_renderTimings.Frame = 0;
	_renderTimings.GBuffer = 0;
	_renderTimings.Lighting = 0;
	_renderTimings.Shadow = 0;
	_renderTimings.Ssao = 0;
}

void Renderer::EndFrame()
{
}

void Renderer::ShadowDepthPass()
{
	auto start = std::chrono::high_resolution_clock::now();

	auto currentViewport = _renderDevice->GetViewport();

	ViewportDesc newViewport;
	newViewport.Width = _desc.ShadowRes.X;
	newViewport.Height = _desc.ShadowRes.Y;
	_renderDevice->SetViewport(newViewport);

	ShadowBufferData data;
	data.Projection = Matrix4::Orthographic(-_shadowOrthographicSize, _shadowOrthographicSize, -_shadowOrthographicSize, _shadowOrthographicSize, -_shadowOrthographicSize, _shadowOrthographicSize);
	data.View = Matrix4::LookAt(-_directionalLightData.Direction, Vector3::Zero, Vector3(0.0f, 1.0f, 0.0f));
	data.TexelDims = Vector2(1.0f / _desc.ShadowRes.X, 1.0f / _desc.ShadowRes.Y);
	_depthBuffer->WriteData(0, sizeof(ShadowBufferData), &data, AccessType::WriteOnlyDiscard);

	_renderDevice->SetPipelineState(_shadowDepthPso);
	_renderDevice->SetRenderTarget(_depthRenderTarget);
	_renderDevice->SetConstantBuffer(3, _depthBuffer);

	_renderDevice->ClearBuffers(RTT_Depth);

	for (auto renderable : _renderables)
	{
		auto mesh = renderable->GetMesh();
		_renderDevice->SetConstantBuffer(0, renderable->GetPerObjectBuffer());
		_renderDevice->SetVertexBuffer(mesh->GetVertexData());

		if (mesh->IsIndexed())
		{
			auto indexCount = mesh->GetIndexCount();
			_renderDevice->SetIndexBuffer(mesh->GetIndexData());
			_renderDevice->DrawIndexed(indexCount, 0, 0);
		}
		else
		{
			_renderDevice->Draw(mesh->GetVertexCount(), 0);
		}
	}

	_renderDevice->SetViewport(currentViewport);

	auto end = std::chrono::high_resolution_clock::now();
	_renderTimings.Shadow = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::SsaoPass()
{
	auto start = std::chrono::high_resolution_clock::now();

	_renderDevice->SetPipelineState(_ssaoPassPso);
	_renderDevice->SetRenderTarget(_ssaoRT);

	_renderDevice->SetTexture(0, _gBuffer->GetColourTarget(0));
	_renderDevice->SetTexture(1, _gBuffer->GetColourTarget(1));
	_renderDevice->SetTexture(2, _ssaoNoiseTexture);

	_renderDevice->SetSamplerState(0, _ssaoSamplerState);
	_renderDevice->SetSamplerState(1, _ssaoSamplerState);
	_renderDevice->SetSamplerState(2, _ssaoSamplerState);

	_renderDevice->SetConstantBuffer(1, _frameBuffer);

	_renderDevice->SetVertexBuffer(_fsQuadBuffer);
	_renderDevice->Draw(6, 0);

	auto end = std::chrono::high_resolution_clock::now();
	_renderTimings.Ssao = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::GBufferFillPass()
{
	auto start = std::chrono::high_resolution_clock::now();

	_renderDevice->SetPipelineState(_gBufferPso);
	_renderDevice->SetRenderTarget(_gBufferRto);
	_renderDevice->ClearBuffers(RTT_Colour | RTT_Depth | RTT_Stencil);

	for (auto renderable : _renderables)
	{
		auto mesh = renderable->GetMesh();
		auto material = mesh->GetMaterial();

		SetMaterialData(material);

		_renderDevice->SetConstantBuffer(0, renderable->GetPerObjectBuffer());
		_renderDevice->SetConstantBuffer(2, _materialBuffer);
		_renderDevice->SetVertexBuffer(mesh->GetVertexData());

		if (mesh->IsIndexed())
		{
			auto indexCount = mesh->GetIndexCount();
			_renderDevice->SetIndexBuffer(mesh->GetIndexData());
			_renderDevice->DrawIndexed(indexCount, 0, 0);

			_renderCounts.TriangleCount += (indexCount * 3);
		}
		else
		{
			_renderDevice->Draw(mesh->GetVertexCount(), 0);

			auto vertexCount = mesh->GetVertexCount();
			_renderCounts.TriangleCount += (vertexCount * 3);
		}
		_renderCounts.DrawCount++;
	}

	auto end = std::chrono::high_resolution_clock::now();
	_renderTimings.GBuffer = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::PointLightPass()
{
	auto start = std::chrono::high_resolution_clock::now();

	_renderDevice->SetRenderTarget(_pointLightRto);
	_renderDevice->ClearBuffers(RTT_Colour);
	_renderDevice->SetTexture(0, _gBufferRto->GetDepthStencilTarget());
	_renderDevice->SetTexture(1, _gBufferRto->GetColourTarget(1));
	_renderDevice->SetTexture(2, _gBufferRto->GetColourTarget(2));
	_renderDevice->SetSamplerState(0, _noMipSamplerState);
	_renderDevice->SetSamplerState(1, _noMipSamplerState);
	_renderDevice->SetSamplerState(2, _noMipSamplerState);
	_renderDevice->SetConstantBuffer(1, _pointLightConstantsBuffer);
	_renderDevice->SetConstantBuffer(2, _pointLightBuffer);

	PointLightConstantsBuffer pointLightConstantsBuffer;
	pointLightConstantsBuffer.ProjViewInvs = (GetBoundCamera()->GetProj() * GetBoundCamera()->GetView()).Inverse();
	pointLightConstantsBuffer.CameraPosition = GetBoundCamera()->GetTransform().GetPosition();
	pointLightConstantsBuffer.PixelSize = Vector2(1.0f / _desc.RenderWidth, 1.0f / _desc.RenderHeight);
	_pointLightConstantsBuffer->WriteData(0, sizeof(PointLightConstantsBuffer), &pointLightConstantsBuffer, AccessType::WriteOnlyDiscard);

	for (auto pointLight : _pointLights)
	{

		float32 distToLight = (pointLight->GetTransform().GetPosition() - GetBoundCamera()->GetTransform().GetPosition()).Length();
		if (distToLight < pointLight->GetRadius())
		{
			_renderDevice->SetPipelineState(_pointLightCWPso);
		}
		else
		{
			_renderDevice->SetPipelineState(_pointLightCCWPso);
		}

		PointLightBuffer pointLightBuffer;
		pointLightBuffer.Colour = pointLight->GetColour().ToVec3();
		pointLightBuffer.Position = pointLight->GetTransform().GetPosition();
		pointLightBuffer.Radius = pointLight->GetRadius();
		_pointLightBuffer->WriteData(0, sizeof(PointLightBuffer), &pointLightBuffer, AccessType::WriteOnlyDiscard);

		PerObjectBufferData perObjectBufferData;
		perObjectBufferData.Model = pointLight->GetWorldTransform().GetMatrix();
		perObjectBufferData.ModelView = GetBoundCamera()->GetView() * perObjectBufferData.Model;
		perObjectBufferData.ModelViewProjection = GetBoundCamera()->GetProj() * perObjectBufferData.ModelView;
		_lightObjectBuffer->WriteData(0, sizeof(PerObjectBufferData), &perObjectBufferData, AccessType::WriteOnlyDiscard);

		_renderDevice->SetConstantBuffer(0, _lightObjectBuffer);
		_renderDevice->SetVertexBuffer(_pointLightMesh->GetVertexData());
		_renderDevice->SetIndexBuffer(_pointLightMesh->GetIndexData());
		_renderDevice->DrawIndexed(_pointLightMesh->GetIndexCount(), 0, 0);
	}

	auto end = std::chrono::high_resolution_clock::now();
	_renderTimings.Lighting = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void Renderer::MergePass()
{
	_renderDevice->SetRenderTarget(nullptr);
	_renderDevice->SetPipelineState(_mergePso);
	_renderDevice->SetTexture(0, _gBufferRto->GetColourTarget(0));
	_renderDevice->SetTexture(1, _pointLightRto->GetColourTarget(0));
	_renderDevice->SetTexture(2, _pointLightRto->GetColourTarget(1));
	_renderDevice->SetSamplerState(0, _noMipSamplerState);
	_renderDevice->SetSamplerState(1, _noMipSamplerState);
	_renderDevice->SetSamplerState(2, _noMipSamplerState);
	_renderDevice->SetVertexBuffer(_fsQuadBuffer);
	_renderDevice->Draw(6, 0);

	auto end = std::chrono::high_resolution_clock::now();
}

void Renderer::SsaoBlurPass()
{
	_renderDevice->SetPipelineState(_ssaoBlurPassPso);
	_renderDevice->SetRenderTarget(_ssaoBlurRT);

	_renderDevice->SetTexture(0, _ssaoRT->GetColourTarget(0));

	_renderDevice->SetSamplerState(0, _ssaoSamplerState);

	_renderDevice->SetVertexBuffer(_fsQuadBuffer);
	_renderDevice->Draw(6, 0);
}

void Renderer::DepthDebugPass(const std::shared_ptr<Texture> &depthTexture)
{
	_renderDevice->SetRenderTarget(nullptr);
	_renderDevice->SetPipelineState(_depthDebugPso);
	_renderDevice->SetTexture(0, depthTexture);
	_renderDevice->SetSamplerState(0, _noMipSamplerState);
	_renderDevice->SetConstantBuffer(1, _frameBuffer);
	_renderDevice->SetVertexBuffer(_fsQuadBuffer);
	_renderDevice->Draw(6, 0);
}

void Renderer::RTOTextureDebugPass(const std::shared_ptr<Texture> &texture)
{
	_renderDevice->SetRenderTarget(nullptr);
	_renderDevice->SetPipelineState(_gBufferDebugPso);
	_renderDevice->SetTexture(0, texture);
	_renderDevice->SetSamplerState(0, _noMipSamplerState);
	_renderDevice->SetVertexBuffer(_fsQuadBuffer);
	_renderDevice->Draw(6, 0);
}

void Renderer::SetMaterialData(const std::shared_ptr<Material> &material)
{
	if (_activeMaterial != nullptr &&
			material->GetAmbientColour() == _activeMaterial->GetAmbientColour() &&
			material->GetDiffuseColour() == _activeMaterial->GetDiffuseColour() &&
			material->GetSpecularColour() == _activeMaterial->GetSpecularColour() &&
			material->GetDiffuseTexture() == _activeMaterial->GetDiffuseTexture() &&
			material->GetNormalTexture() == _activeMaterial->GetNormalTexture() &&
			material->GetSpecularTexture() == _activeMaterial->GetSpecularTexture() &&
			material->GetOpacityTexture() == _activeMaterial->GetOpacityTexture())
	{
		return;
	}

	MaterialBufferData matData;
	matData.Ambient = material->GetAmbientColour();
	matData.Diffuse = material->GetDiffuseColour();
	matData.Specular = material->GetSpecularColour();
	matData.Exponent = material->GetSpecularExponent();

	auto diffuseTexture = material->GetDiffuseTexture();
	if (diffuseTexture)
	{
		matData.EnabledTextureMaps.Diffuse = 1;
		_renderDevice->SetTexture(0, diffuseTexture);
		_renderDevice->SetSamplerState(0, _basicSamplerState);
	}

	auto normalTexture = material->GetNormalTexture();
	if (normalTexture)
	{
		matData.EnabledTextureMaps.Normal = 1;
		_renderDevice->SetTexture(1, normalTexture);
		_renderDevice->SetSamplerState(1, _noMipSamplerState);
	}

	auto specularTexture = material->GetSpecularTexture();
	if (specularTexture)
	{
		matData.EnabledTextureMaps.Specular = 1;
		_renderDevice->SetTexture(2, specularTexture);
		_renderDevice->SetSamplerState(2, _noMipSamplerState);
	}

	auto opacityTexture = material->GetOpacityTexture();
	if (opacityTexture)
	{
		matData.EnabledTextureMaps.Opacity = 1;
		_renderDevice->SetTexture(3, opacityTexture);
		_renderDevice->SetSamplerState(3, _noMipSamplerState);
	}

	_materialBuffer->WriteData(0, sizeof(MaterialBufferData), &matData, AccessType::WriteOnlyDiscard);
	_activeMaterial = material;

	_renderCounts.MaterialCount++;
}

#include "UiManager.hpp"

#include <memory>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../Core/Types.hpp"
#include "../Core/Scene.h"
#include "../Image/ImageData.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Maths/Radian.hpp"
#include "../RenderApi/GL/GLTexture.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../Utility/String.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

bool show_demo_window = false;
bool lockCameraToLight = false;

std::unordered_map<uint64, std::shared_ptr<Texture>> UiManager::TEXTURE_MAP;

void UiManager::addTexture(uint64 id, const std::shared_ptr<Texture> &texture)
{
	TEXTURE_MAP[id] = texture;
}

UiManager::UiManager(GLFWwindow *glfwWindow) : _io(nullptr),
																							 _vertBuffSize(0),
																							 _idxBuffSize(0),
																							 _initialized(false)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	_io = &ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
	ImGui_ImplOpenGL3_Init();
}

UiManager::~UiManager()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool UiManager::hasMouseCapture() const
{
	return _io->WantCaptureMouse;
}

void UiManager::update(Scene &scene)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		bool displayDebugWindow = true;
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		if (ImGui::Begin("Scene", &displayDebugWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
		{
			ImGui::Separator();
			{
				scene.drawDebugUi();
			}

			ImGui::Separator();
			{
				ImGui::Checkbox("Demo Window", &show_demo_window);
			}

			ImGui::Separator();
			{
				ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
			}

			if (show_demo_window)
			{
				ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
				ImGui::ShowDemoWindow(&show_demo_window);
			}
		}
		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	draw(ImGui::GetDrawData());
}

void UiManager::draw(ImDrawData *drawData)
{
	auto currentScissorDim = _renderDevice->getScissorDimensions();
	auto currentViewport = _renderDevice->getViewport();

	int32 fbWidth = (int32)(drawData->DisplaySize.x * _io->DisplayFramebufferScale.x);
	int32 fbHeight = (int32)(drawData->DisplaySize.y * _io->DisplayFramebufferScale.y);
	if (fbWidth <= 0 || fbHeight <= 0)
	{
		return;
	}
	drawData->ScaleClipRects(_io->DisplayFramebufferScale);

	float32 L = drawData->DisplayPos.x;
	float32 R = drawData->DisplayPos.x + drawData->DisplaySize.x;
	float32 T = drawData->DisplayPos.y;
	float32 B = drawData->DisplayPos.y + drawData->DisplaySize.y;
	auto orthProj = Matrix4::Orthographic(L, R, B, T, -1.0f, 1.0f);

	ViewportDesc newViewport;
	newViewport.TopLeftX = 0;
	newViewport.TopLeftY = 0;
	newViewport.Width = fbWidth;
	newViewport.Height = fbHeight;
	_renderDevice->setViewport(newViewport);

	GpuBufferDesc constBufferDesc;
	constBufferDesc.BufferType = BufferType::Constant;
	constBufferDesc.BufferUsage = BufferUsage::Dynamic;
	constBufferDesc.ByteCount = sizeof(Matrix4);
	_constBuffer = _renderDevice->createGpuBuffer(constBufferDesc);
	_constBuffer->writeData(0, constBufferDesc.ByteCount, &orthProj[0][0], AccessType::WriteOnlyDiscardRange);

	if (!_vertBuffer || _vertBuffSize < drawData->TotalVtxCount)
	{
		_vertBuffSize = drawData->TotalVtxCount + 5000;

		VertexBufferDesc vertBufferDesc;
		vertBufferDesc.BufferUsage = BufferUsage::Dynamic;
		vertBufferDesc.VertexCount = _vertBuffSize;
		vertBufferDesc.VertexSizeBytes = sizeof(ImDrawVert);
		_vertBuffer = _renderDevice->createVertexBuffer(vertBufferDesc);
	}

	if (!_idxBuffer || _idxBuffSize < drawData->TotalIdxCount)
	{
		_idxBuffSize = drawData->TotalIdxCount + 10000;

		IndexBufferDesc idxBufferDesc;
		idxBufferDesc.BufferUsage = BufferUsage::Default;
		idxBufferDesc.IndexCount = _idxBuffSize;
		idxBufferDesc.IndexType = sizeof(ImDrawIdx) == 2 ? IndexType::UInt16 : IndexType::UInt32;
		_idxBuffer = _renderDevice->createIndexBuffer(idxBufferDesc);
	}

	uint64 vertByteOffset = 0;
	uint64 idxByteOffset = 0;
	for (int32 n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList *cmdList = drawData->CmdLists[n];
		uint64 vertByteCount = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
		uint32 idxByteCount = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

		_vertBuffer->writeData(vertByteOffset, vertByteCount, cmdList->VtxBuffer.Data);
		_idxBuffer->writeData(idxByteOffset, idxByteCount, cmdList->IdxBuffer.Data);

		vertByteOffset += vertByteCount;
		idxByteOffset += idxByteCount;
	}

	uint64 vertOffset = 0;
	uint64 idxOffset = 0;
	ImVec2 pos = drawData->DisplayPos;
	for (int32 n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList *cmdList = drawData->CmdLists[n];
		for (int32 i = 0; i < cmdList->CmdBuffer.size(); i++)
		{
			const ImDrawCmd *pCmd = &cmdList->CmdBuffer[i];
			ImVec4 clipRect(pCmd->ClipRect.x - pos.x, pCmd->ClipRect.y - pos.y, pCmd->ClipRect.z - pos.x, pCmd->ClipRect.w - pos.y);

			ScissorDesc newScissorDim;
			newScissorDim.X = clipRect.x;
			newScissorDim.Y = _renderDevice->getRenderHeight() - clipRect.w;
			newScissorDim.W = clipRect.z - clipRect.x;
			newScissorDim.H = clipRect.w - clipRect.y;
			_renderDevice->setScissorDimensions(newScissorDim);

			auto texture = TEXTURE_MAP[reinterpret_cast<uint64>(pCmd->TextureId)];
			if (texture)
			{
				_renderDevice->setTexture(0, texture);
			}
			else
			{
				_renderDevice->setTexture(0, _textureAtlas);
			}

			_renderDevice->setPipelineState(_pipelineState);
			_renderDevice->setSamplerState(0, _noMipSamplerState);

			_renderDevice->setVertexBuffer(_vertBuffer);
			_renderDevice->setIndexBuffer(_idxBuffer);
			_renderDevice->setConstantBuffer(0, _constBuffer);

			_renderDevice->drawIndexed(pCmd->ElemCount, idxOffset, vertOffset);
			idxOffset += pCmd->ElemCount;
		}
		vertOffset += cmdList->VtxBuffer.Size;
	}

	_renderDevice->setScissorDimensions(currentScissorDim);
	_renderDevice->setViewport(currentViewport);
}

void UiManager::initialize(std::shared_ptr<RenderDevice> renderDevice)
{
	if (!_initialized)
	{
		_renderDevice = renderDevice;

		setupRenderer();
		setupFontAtlas();
		_initialized = true;
	}
}

void UiManager::setupRenderer()
{

	ShaderDesc vsShaderDesc;
	vsShaderDesc.ShaderType = ShaderType::Vertex;
	vsShaderDesc.Source = String::foadFromFile("./Shaders/Editor/UiElements.vert");

	ShaderDesc psShaderDesc;
	psShaderDesc.ShaderType = ShaderType::Fragment;
	psShaderDesc.Source = String::foadFromFile("./Shaders/Editor/UiElements.frag");

	BlendStateDesc blendStateDesc;
	blendStateDesc.RTBlendState[0].BlendEnabled = true;
	blendStateDesc.RTBlendState[0].Blend = BlendDesc(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha, BlendOperation::Add);

	DepthStencilStateDesc depthStencilStateDesc;
	depthStencilStateDesc.DepthReadEnabled = false;

	RasterizerStateDesc rasterizerStateDesc;
	rasterizerStateDesc.CullMode = CullMode::None;
	rasterizerStateDesc.ScissorEnabled = true;
	rasterizerStateDesc.FillMode = FillMode::Solid;

	std::vector<VertexLayoutDesc> vertexLayoutDesc{
			VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
			VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
			VertexLayoutDesc(SemanticType::Colour, SemanticFormat::Ubyte4, true)};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->addParam(ShaderParam("Constants", ShaderParamType::ConstBuffer, 0));
	shaderParams->addParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));

	try
	{
		PipelineStateDesc pStateDesc;
		pStateDesc.BlendState = _renderDevice->createBlendState(blendStateDesc);
		pStateDesc.DepthStencilState = _renderDevice->createDepthStencilState(depthStencilStateDesc);
		pStateDesc.RasterizerState = _renderDevice->createRasterizerState(rasterizerStateDesc);
		pStateDesc.VS = _renderDevice->createShader(vsShaderDesc);
		pStateDesc.FS = _renderDevice->createShader(psShaderDesc);
		pStateDesc.VertexLayout = _renderDevice->createVertexLayout(vertexLayoutDesc);
		pStateDesc.ShaderParams = shaderParams;

		_pipelineState = _renderDevice->createPipelineState(pStateDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize debug UI pipeline state. " + std::string(exception.what()));
	}

	try
	{
		SamplerStateDesc samplerStateDesc;
		samplerStateDesc.MinFiltering = TextureFilteringMode::Linear;
		samplerStateDesc.MaxFiltering = TextureFilteringMode::Linear;
		samplerStateDesc.MipFiltering = TextureFilteringMode::Linear;
		_samplerState = _renderDevice->createSamplerState(samplerStateDesc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize UI sampler state. " + std::string(exception.what()));
	}

	SamplerStateDesc desc;
	desc.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
	desc.MinFiltering = TextureFilteringMode::None;
	desc.MinFiltering = TextureFilteringMode::None;
	desc.MipFiltering = TextureFilteringMode::None;
	try
	{
		_noMipSamplerState = _renderDevice->createSamplerState(desc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize UI sampler state. " + std::string(exception.what()));
	}
}

void UiManager::setupFontAtlas()
{
	ubyte *pixels = nullptr;
	int32 width = 0;
	int32 height = 0;
	_io->Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	TextureDesc desc;
	desc.Format = TextureFormat::RGBA8;
	desc.Type = TextureType::Texture2D;
	desc.Width = static_cast<uint32>(width);
	desc.Height = static_cast<uint32>(height);

	_textureAtlas = _renderDevice->createTexture(desc);

	std::shared_ptr<ImageData> imageData(new ImageData(desc.Width, desc.Height, 1, ImageFormat::RGBA8));
	imageData->writeData(pixels);
	_textureAtlas->writeData(0, 0, imageData);
	_textureAtlas->generateMips();
	addTexture(reinterpret_cast<uint64>(&_textureAtlas), _textureAtlas);

	_io->Fonts->TexID = &_textureAtlas;
}

void UiManager::drawDrawables(const std::vector<Drawable> &drawables)
{
}
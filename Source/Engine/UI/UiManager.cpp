#include "UiManager.hpp"

#include <memory>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "UiInspector.hpp"
#include "../Core/Types.hpp"
#include "../Image/ImageData.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Maths/Radian.hpp"
#include "../RenderApi/GL/GLTexture.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../Rendering/Material.hpp"
#include "../Rendering/Renderer.h"
#include "../Rendering/Renderable.hpp"
#include "../Rendering/StaticMesh.h"
#include "../SceneManagement/SceneNode.hpp"
#include "../SceneManagement/Transform.h"
#include "../Utility/String.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

bool show_demo_window = false;
bool lockCameraToLight = false;
uint32 selectedActorIndex = -1;
std::shared_ptr<SceneNode> selectedActor = nullptr;

std::unordered_map<uint64, std::shared_ptr<Texture>> UiManager::TEXTURE_MAP;

void UiManager::AddTexture(uint64 id, const std::shared_ptr<Texture> &texture)
{
	TEXTURE_MAP[id] = texture;
}

UiManager::UiManager(GLFWwindow *glfwWindow) : _io(nullptr),
																							 _window(glfwWindow),
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

void UiManager::SetRenderer(const std::shared_ptr<Renderer> &renderer)
{
	_renderer = renderer;
}

void UiManager::SetSceneGraph(const std::shared_ptr<SceneNode> &sceneGraph)
{
	_sceneGraph = sceneGraph;
}

bool UiManager::HasMouseCapture() const
{
	return _io->WantCaptureMouse;
}

void UiManager::Update()
{
	if (!_initialized)
	{
		SetupRenderer();
		SetupFontAtlas();
		_initialized = true;
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{

		bool displayDebugWindow = true;
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		if (!ImGui::Begin("Debug", &displayDebugWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
		{
			ImGui::End();
			return;
		}

		ImGui::Separator();
		if (ImGui::TreeNode("Scene"))
		{
			ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvail().x, 200), false, ImGuiWindowFlags_HorizontalScrollbar);
			DrawSceneNode(_sceneGraph);
			UiInspector::Build(selectedActor);

			ImGui::EndChild();
			ImGui::TreePop();
		}

		ImGui::Separator();
		{
			ImGui::Checkbox("Demo Window", &show_demo_window);
		}

		ImGui::Separator();
		{
			ImGui::Text("Post Processing");

			bool hdrEnabled = _renderer->IsHdrEnabled();
			bool ssaoEnabled = _renderer->IsSsaoEnabled();

			ImGui::Checkbox("HDR", &hdrEnabled);
			ImGui::Checkbox("SSAO", &ssaoEnabled);

			_renderer->EnableHdr(hdrEnabled);
			_renderer->EnableSsao(ssaoEnabled);
		}

		{
			std::vector<const char *> debugRenderingItems = {"Disabled", "Diffuse", "Normal", "Depth", "Emissive", "Specular"};
			static int debugRenderingCurrentItem = 0;
			ImGui::Combo("Debug Rendering", &debugRenderingCurrentItem, debugRenderingItems.data(), debugRenderingItems.size());
			_renderer->EnableDebugPass(static_cast<DebugDisplayType>(debugRenderingCurrentItem));
			ImGui::Separator();
		}

		ImGui::Separator();
		{
			ImGui::Text("Renderer Settings");

			float32 currentSize = _renderer->GetOrthographicSize();
			ImGui::DragFloat("Shadow Projection Size", &currentSize);
			_renderer->SetOrthographicSize(currentSize);
		}

		{
			auto frameTimings = _renderer->GetFrameRenderTimings();
			ImGui::Text("Render Pass");
			ImGui::BulletText("All %.3f ms", static_cast<float32>(frameTimings.Frame * 1e-6f));
			ImGui::BulletText("Shadow %.3f ms", static_cast<float32>(frameTimings.Shadow * 1e-6f));
			ImGui::BulletText("G-Buffer %.3f ms", static_cast<float32>(frameTimings.GBuffer * 1e-6f));
			ImGui::BulletText("Lighting %.3f ms", static_cast<float32>(frameTimings.Lighting * 1e-6f));
			ImGui::BulletText("SSAO %.3f ms", static_cast<float32>(frameTimings.Ssao * 1e-6f));
		}

		ImGui::Separator();
		{
			std::stringstream drawCounts;
			auto frameCounts = _renderer->GetFrameRenderCounts();
			drawCounts << frameCounts.DrawCount << " draw calls\n"
								 << frameCounts.TriangleCount << " triangles\n"
								 << frameCounts.MaterialCount << " materials\n";
			ImGui::Text(drawCounts.str().c_str());

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		ImGui::End();
	}

	if (show_demo_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	ImGui::EndFrame();
	ImGui::Render();
	Draw(ImGui::GetDrawData());
}

void UiManager::Draw(ImDrawData *drawData)
{
	auto renderDevice = Renderer::GetRenderDevice();
	auto currentScissorDim = renderDevice->GetScissorDimensions();
	auto currentViewport = renderDevice->GetViewport();

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
	renderDevice->SetViewport(newViewport);

	GpuBufferDesc constBufferDesc;
	constBufferDesc.BufferType = BufferType::Constant;
	constBufferDesc.BufferUsage = BufferUsage::Dynamic;
	constBufferDesc.ByteCount = sizeof(Matrix4);
	_constBuffer = renderDevice->CreateGpuBuffer(constBufferDesc);
	_constBuffer->WriteData(0, constBufferDesc.ByteCount, &orthProj[0][0], AccessType::WriteOnlyDiscardRange);

	if (!_vertBuffer || _vertBuffSize < drawData->TotalVtxCount)
	{
		_vertBuffSize = drawData->TotalVtxCount + 5000;

		VertexBufferDesc vertBufferDesc;
		vertBufferDesc.BufferUsage = BufferUsage::Dynamic;
		vertBufferDesc.VertexCount = _vertBuffSize;
		vertBufferDesc.VertexSizeBytes = sizeof(ImDrawVert);
		_vertBuffer = renderDevice->CreateVertexBuffer(vertBufferDesc);
	}

	if (!_idxBuffer || _idxBuffSize < drawData->TotalIdxCount)
	{
		_idxBuffSize = drawData->TotalIdxCount + 10000;

		IndexBufferDesc idxBufferDesc;
		idxBufferDesc.BufferUsage = BufferUsage::Default;
		idxBufferDesc.IndexCount = _idxBuffSize;
		idxBufferDesc.IndexType = sizeof(ImDrawIdx) == 2 ? IndexType::UInt16 : IndexType::UInt32;
		_idxBuffer = renderDevice->CreateIndexBuffer(idxBufferDesc);
	}

	uint64 vertByteOffset = 0;
	uint64 idxByteOffset = 0;
	for (int32 n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList *cmdList = drawData->CmdLists[n];
		uint64 vertByteCount = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
		uint32 idxByteCount = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

		_vertBuffer->WriteData(vertByteOffset, vertByteCount, cmdList->VtxBuffer.Data);
		_idxBuffer->WriteData(idxByteOffset, idxByteCount, cmdList->IdxBuffer.Data);

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
			newScissorDim.Y = renderDevice->GetRenderHeight() - clipRect.w;
			newScissorDim.W = clipRect.z - clipRect.x;
			newScissorDim.H = clipRect.w - clipRect.y;
			renderDevice->SetScissorDimensions(newScissorDim);

			auto texture = TEXTURE_MAP[reinterpret_cast<uint64>(pCmd->TextureId)];
			if (texture)
			{
				renderDevice->SetTexture(0, texture);
			}
			else
			{
				renderDevice->SetTexture(0, _textureAtlas);
			}

			renderDevice->SetPipelineState(_pipelineState);
			renderDevice->SetSamplerState(0, _noMipSamplerState);

			renderDevice->SetVertexBuffer(_vertBuffer);
			renderDevice->SetIndexBuffer(_idxBuffer);
			renderDevice->SetConstantBuffer(0, _constBuffer);

			renderDevice->DrawIndexed(pCmd->ElemCount, idxOffset, vertOffset);
			idxOffset += pCmd->ElemCount;
		}
		vertOffset += cmdList->VtxBuffer.Size;
	}

	renderDevice->SetScissorDimensions(currentScissorDim);
	renderDevice->SetViewport(currentViewport);
}

void UiManager::SetupRenderer()
{
	ShaderDesc vsShaderDesc;
	vsShaderDesc.EntryPoint = "main";
	vsShaderDesc.ShaderLang = ShaderLang::Glsl;
	vsShaderDesc.ShaderType = ShaderType::Vertex;
	vsShaderDesc.Source = String::LoadFromFile("./Shaders/DebugGui.vert");

	ShaderDesc psShaderDesc;
	psShaderDesc.EntryPoint = "main";
	psShaderDesc.ShaderLang = ShaderLang::Glsl;
	psShaderDesc.ShaderType = ShaderType::Pixel;
	psShaderDesc.Source = String::LoadFromFile("./Shaders/DebugGui.frag");

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
	shaderParams->AddParam(ShaderParam("Constants", ShaderParamType::ConstBuffer, 0));
	shaderParams->AddParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));

	auto renderDevice = Renderer::GetRenderDevice();
	try
	{
		PipelineStateDesc pStateDesc;
		pStateDesc.BlendState = renderDevice->CreateBlendState(blendStateDesc);
		pStateDesc.DepthStencilState = renderDevice->CreateDepthStencilState(depthStencilStateDesc);
		pStateDesc.RasterizerState = renderDevice->CreateRasterizerState(rasterizerStateDesc);
		pStateDesc.VS = renderDevice->CreateShader(vsShaderDesc);
		pStateDesc.PS = renderDevice->CreateShader(psShaderDesc);
		pStateDesc.VertexLayout = renderDevice->CreateVertexLayout(vertexLayoutDesc);
		pStateDesc.ShaderParams = shaderParams;

		_pipelineState = renderDevice->CreatePipelineState(pStateDesc);
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
		_samplerState = renderDevice->CreateSamplerState(samplerStateDesc);
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
		_noMipSamplerState = renderDevice->CreateSamplerState(desc);
	}
	catch (const std::exception &exception)
	{
		throw std::runtime_error("Unable to initialize UI sampler state. " + std::string(exception.what()));
	}
}

void UiManager::SetupFontAtlas()
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

	auto renderDevice = Renderer::GetRenderDevice();
	_textureAtlas = renderDevice->CreateTexture(desc);

	std::shared_ptr<ImageData> imageData(new ImageData(desc.Width, desc.Height, 1, ImageFormat::RGBA8));
	imageData->WriteData(pixels);
	_textureAtlas->WriteData(0, 0, imageData);
	_textureAtlas->GenerateMips();
	AddTexture(reinterpret_cast<uint64>(&_textureAtlas), _textureAtlas);

	_io->Fonts->TexID = &_textureAtlas;
}

void UiManager::DrawSceneNode(const sptr<SceneNode> &parentNode)
{
	auto childNodes = parentNode->GetAllChildNodes();
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (selectedActor == parentNode ? ImGuiTreeNodeFlags_Selected : 0);

	if (childNodes.empty())
	{
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		ImGui::TreeNodeEx(parentNode->GetName().c_str(), flags);
		if (ImGui::IsItemClicked())
		{
			selectedActor = parentNode;
		}
	}
	else
	{
		bool open = ImGui::TreeNodeEx(parentNode->GetName().c_str(), flags);
		if (ImGui::IsItemClicked())
		{
			selectedActor = parentNode;
		}
		if (open)
		{
			for (auto childNode : childNodes)
			{
				DrawSceneNode(childNode);
			}
			ImGui::TreePop();
		}
	}
}

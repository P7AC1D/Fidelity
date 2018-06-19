#include "DebugUi.hpp"

#include <SDL.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"
#include "../Core/Types.hpp"
#include "../Image/ImageData.hpp"
#include "../Maths/Matrix4.hpp"
#include "../RenderApi/GL/GLTexture.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../Rendering/Renderer.h"
#include "../Utility/String.hpp"

bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

DebugUi::DebugUi(SDL_Window* sdlWindow, SDL_GLContext sdlGlContext):
	_io(nullptr),
	_sdlWindow(sdlWindow),
	_sdlGlContext(sdlGlContext),
	_vertBuffSize(0),
	_idxBuffSize(0),
	_initialized(false)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	_io = &ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(_sdlWindow, sdlGlContext);
}

DebugUi::~DebugUi()
{
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void DebugUi::SetCamera(const std::shared_ptr<Camera>& camera)
{
	_camera = camera;
}

void DebugUi::ProcessEvents(SDL_Event* sdlEvent)
{
	ImGui_ImplSDL2_ProcessEvent(sdlEvent);
}

void DebugUi::Update()
{
	if (!_initialized)
	{
		SetupRenderer();
		SetupFontAtlas();
		_initialized = true;
	}

	ImGui_ImplSDL2_NewFrame(_sdlWindow);
	ImGui::NewFrame();
	{
		static float f = 0.0f;
		static int counter = 0;
		ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
	if (show_demo_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	ImGui::EndFrame();
	ImGui::Render();
	SDL_GL_MakeCurrent(_sdlWindow, _sdlGlContext);
	Draw(ImGui::GetDrawData());
}

void DebugUi::Draw(ImDrawData* drawData)
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
		const ImDrawList* cmdList = drawData->CmdLists[n];
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
		const ImDrawList* cmdList = drawData->CmdLists[n];		
		for (int32 i = 0; i < cmdList->CmdBuffer.size(); i++)
		{
			const ImDrawCmd* pCmd = &cmdList->CmdBuffer[i];
			ImVec4 clipRect(pCmd->ClipRect.x - pos.x, pCmd->ClipRect.y - pos.y, pCmd->ClipRect.z - pos.x, pCmd->ClipRect.w - pos.y);

			ScissorDesc newScissorDim;
			newScissorDim.X = clipRect.x;
			newScissorDim.Y = renderDevice->GetRenderHeight() - clipRect.w;
			newScissorDim.W = clipRect.z - clipRect.x;
			newScissorDim.H = clipRect.w - clipRect.y;
			renderDevice->SetScissorDimensions(newScissorDim);

			renderDevice->SetPipelineState(_pipelineState);
			renderDevice->SetTexture(0, _textureAtlas);
			renderDevice->SetSamplerState(0, _samplerState);
			renderDevice->SetVertexBuffer(0, _vertBuffer);
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

void DebugUi::SetupRenderer()
{
	ShaderDesc vsShaderDesc;
	vsShaderDesc.EntryPoint = "main";
	vsShaderDesc.ShaderLang = ShaderLang::Glsl;
	vsShaderDesc.ShaderType = ShaderType::Vertex;
	vsShaderDesc.Source = String::LoadFromFile("./../../Resources/Shaders/DebugGuiVS.glsl");

	ShaderDesc psShaderDesc;
	psShaderDesc.EntryPoint = "main";
	psShaderDesc.ShaderLang = ShaderLang::Glsl;
	psShaderDesc.ShaderType = ShaderType::Pixel;
	psShaderDesc.Source = String::LoadFromFile("./../../Resources/Shaders/DebugGuiPS.glsl");

	BlendStateDesc blendStateDesc;
	blendStateDesc.RTBlendState[0].BlendEnabled = true;
	blendStateDesc.RTBlendState[0].Blend = BlendDesc(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha, BlendOperation::Add);

	DepthStencilStateDesc depthStencilStateDesc;
	depthStencilStateDesc.DepthReadEnabled = false;

	RasterizerStateDesc rasterizerStateDesc;
	rasterizerStateDesc.CullMode = CullMode::None;
	rasterizerStateDesc.ScissorEnabled = true;
	rasterizerStateDesc.FillMode = FillMode::Solid;

	std::vector<VertexLayoutDesc> vertexLayoutDesc
	{
		VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
		VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
		VertexLayoutDesc(SemanticType::Colour, SemanticFormat::Ubyte4, true)
	};

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
	catch (const std::exception& exception)
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
	catch (const std::exception& exception)
	{
		throw std::runtime_error("Unable to initialize UI sampler state. " + std::string(exception.what()));
	}
}

void DebugUi::SetupFontAtlas()
{
	ubyte* pixels = nullptr;
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

	auto glTexture = std::static_pointer_cast<GLTexture>(_textureAtlas);
	_io->Fonts->TexID = (void *)(intptr_t)glTexture->GetId();
}
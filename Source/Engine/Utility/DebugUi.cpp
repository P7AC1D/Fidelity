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

DebugUi::DebugUi(SDL_Window* sdlWindow, SDL_GLContext sdlGlContext): _io(nullptr), _sdlWindow(sdlWindow), _initialized(false)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	_io = &ImGui::GetIO();
	ImGui::StyleColorsDark();

}

void DebugUi::SetCamera(const std::shared_ptr<Camera>& camera)
{
	_camera = camera;
}

void DebugUi::ProcessEvents(SDL_Event* sdlEvent)
{
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
		static float32 f = 0.0f;
		static int32 counter = 0;
		ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	ImGui::EndFrame();
	ImGui::Render();
	Draw(ImGui::GetDrawData());
}

void DebugUi::Draw(ImDrawData* drawData)
{
	auto renderDevice = Renderer::GetRenderDevice();
	auto currentScissorDim = renderDevice->GetScissorDimensions();

	float32 L = drawData->DisplayPos.x;
	float32 R = drawData->DisplayPos.x + drawData->DisplaySize.x;
	float32 T = drawData->DisplayPos.y;
	float32 B = drawData->DisplayPos.y + drawData->DisplaySize.y;
	const float32 ortho_projection[4][4] =
	{
		{ 2.0f / (R - L),    0.0f,              0.0f,   0.0f },
		{ 0.0f,						   2.0f / (T - B),    0.0f,   0.0f },
		{ 0.0f,              0.0f,             -1.0f,   0.0f },
		{ (R + L) / (L - R), (T + B) / (B - T), 0.0f,   1.0f },
	};

	GpuBufferDesc constBufferDesc;
	constBufferDesc.BufferType = BufferType::Constant;
	constBufferDesc.BufferUsage = BufferUsage::Dynamic;
	constBufferDesc.ByteCount = 16 * sizeof(float32);
	_constBuffer = renderDevice->CreateGpuBuffer(constBufferDesc);
	_constBuffer->WriteData(0, constBufferDesc.ByteCount, ortho_projection, AccessType::WriteOnlyDiscardRange);

	for (int32 n = 0; n < drawData->CmdListsCount; n++)
	{
		uint32 indxBufferOffset = 0;
		const ImDrawList* cmdList = drawData->CmdLists[n];

		VertexBufferDesc vertBufferDesc;
		vertBufferDesc.BufferUsage = BufferUsage::Dynamic;
		vertBufferDesc.VertexCount = cmdList->VtxBuffer.size();
		vertBufferDesc.VertexSizeBytes = sizeof(ImDrawVert);
		auto vertBuffer = renderDevice->CreateVertexBuffer(vertBufferDesc);
		vertBuffer->WriteData(0, vertBufferDesc.VertexSizeBytes * vertBufferDesc.VertexCount, cmdList->VtxBuffer.Data);

		IndexBufferDesc idxBufferDesc;
		idxBufferDesc.BufferUsage = BufferUsage::Default;
		idxBufferDesc.IndexCount = cmdList->IdxBuffer.size();
		idxBufferDesc.IndexType = IndexType::UInt16;
		auto idxBuffer = renderDevice->CreateIndexBuffer(idxBufferDesc);
		idxBuffer->WriteData(0, idxBufferDesc.IndexCount * sizeof(uint16), cmdList->IdxBuffer.Data);

		ImVec2 pos = drawData->DisplayPos;
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
			renderDevice->SetVertexBuffer(0, vertBuffer);
			renderDevice->SetIndexBuffer(idxBuffer);
			renderDevice->SetConstantBuffer(0, _constBuffer);

			renderDevice->DrawIndexed(pCmd->ElemCount, indxBufferOffset, 0);
			indxBufferOffset += pCmd->ElemCount;
		}		
	}

	renderDevice->SetScissorDimensions(currentScissorDim);
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
		VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
		VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
		VertexLayoutDesc(SemanticType::Colour, SemanticFormat::Float4)
	};

	std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
	shaderParams->AddParam(ShaderParam("Constants", ShaderParamType::ConstBuffer, 0));
	shaderParams->AddParam(ShaderParam("DiffuseMap", ShaderParamType::Texture, 0));

	try
	{
		auto renderDevice = Renderer::GetRenderDevice();
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

	auto glTexture = std::static_pointer_cast<GLTexture>(_textureAtlas);
	_io->Fonts->TexID = (void *)(intptr_t)glTexture->GetId();
}
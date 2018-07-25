#include "DebugUi.hpp"

#include <memory>
#include <sstream>
#include <SDL.h>
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
#include "../SceneManagement/Actor.hpp"
#include "../SceneManagement/Camera.hpp"
#include "../SceneManagement/SceneManager.h"
#include "../SceneManagement/SceneNode.hpp"
#include "../SceneManagement/Transform.h"
#include "../Utility/String.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl.h"

bool show_demo_window = false;
uint32 selectedActorIndex = -1;
std::shared_ptr<Actor> selectedActor = nullptr;

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

void DebugUi::SetRenderer(const std::shared_ptr<Renderer>& renderer)
{
	_renderer = renderer;
}

bool DebugUi::ProcessEvents(SDL_Event* sdlEvent)
{
	if (_io->WantCaptureMouse)
	{
		return ImGui_ImplSDL2_ProcessEvent(sdlEvent);
	}
	return false;
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
		auto sceneManager = SceneManager::Get();

		bool displayDebugWindow = true;
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		if (!ImGui::Begin("Debug", &displayDebugWindow, ImGuiWindowFlags_AlwaysAutoResize | 
																										ImGuiWindowFlags_NoResize |
																										ImGuiWindowFlags_NoSavedSettings |
																										ImGuiWindowFlags_NoMove))
		{
			ImGui::End();
			return;
		}		

		if (ImGui::TreeNode("Camera"))
		{
			auto camera = sceneManager->GetCamera();

			auto position = camera->GetPosition();
			float32 camPos[3] = { position.X, position.Y, position.Z };
			ImGui::InputFloat3("Position", camPos, 3, ImGuiInputTextFlags_ReadOnly);

			auto target = camera->GetTarget();
			float32 camTar[3] = { target.X, target.Y, target.Z };
			ImGui::InputFloat3("Target", camTar, 3, ImGuiInputTextFlags_ReadOnly);

			std::array<Radian, 3> euler = camera->GetOrientation().ToEuler();
			float32 angles[3] = { euler[0].InDegrees(), euler[1].InDegrees(), euler[2].InDegrees() };
			ImGui::InputFloat3("Orientation", angles, 1, ImGuiInputTextFlags_ReadOnly);

			ImGui::TreePop();
		}

    if (ImGui::TreeNode("Ambient Light"))
    {
      auto colour = sceneManager->GetAmbientLightColour();
      float32 col[3] = { colour[0], colour[1], colour[2] };
      ImGui::ColorEdit3("Colour", col);
			sceneManager->SetAmbientLightColour(Colour(col[0] * 255, col[1] * 255, col[2] * 255));

      auto intensity = sceneManager->GetAmbientLightIntensity();
      ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f);
			sceneManager->SetAmbientLightIntensity(intensity);

      ImGui::TreePop();
    }

		if (ImGui::TreeNode("Directional Light"))
		{
			auto dirLight = sceneManager->GetDirectionalLight();

			auto colour = dirLight->GetColour();
			float32 col[3] = { colour[0], colour[1], colour[2] };
			ImGui::ColorEdit3("Colour", col);
			dirLight->SetColour(Colour(col[0] * 255, col[1] * 255, col[2] * 255));

			auto direction = dirLight->GetDirection();
			float32 dir[3] = { direction[0], direction[1], direction[2] };
			ImGui::SliderFloat3("Direction", dir, -1.0f, 1.0f);
			dirLight->SetDirection(Vector3(dir[0], dir[1], dir[2]));

			auto intensity = dirLight->GetIntensity();
			ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f);
			dirLight->SetIntensity(intensity);

			ImGui::TreePop();
		}
    
    ImGui::Separator();
    if (ImGui::TreeNode("Scene"))
    {
      ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvailWidth(), 200), false, ImGuiWindowFlags_HorizontalScrollbar);
      
      auto sceneGraph = SceneManager::Get()->GetRootSceneNode();
      AddChildNodes(sceneGraph->GetChildNodes());
      AddChildActors(sceneGraph->GetActors());
     
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
			const char* gBufferDebugItems[] = { "Disabled", "Position", "Normal", "Albedo" };
			static int gBufferDebugCurrentItem = 0;
			ImGui::Combo("G-Buffer Debug", &gBufferDebugCurrentItem, gBufferDebugItems, 4);
			_renderer->EnableGBufferDebugPass(static_cast<GBufferDisplayType>(gBufferDebugCurrentItem));
			ImGui::Separator();
		}

    {
      auto frameTimings = _renderer->GetFrameRenderTimings();
			ImGui::Text("Render Pass");
      ImGui::BulletText("All %.3f ms", static_cast<float32>(frameTimings.Frame * 1e-6f));
      ImGui::BulletText("G-Buffer %.3f ms", static_cast<float32>(frameTimings.GBuffer * 1e-6f));
      ImGui::BulletText("SSAO %.3f ms", static_cast<float32>(frameTimings.Ssao * 1e-6f));
      ImGui::BulletText("Lighting %.3f ms", static_cast<float32>(frameTimings.Lighting * 1e-6f));
    }

		ImGui::Separator();
		{
			std::stringstream drawCounts;
			auto frameCounts = _renderer->GetFrameRenderCounts();
			drawCounts << frameCounts.DrawCount << " draw calls\n" << frameCounts.TriangleCount << " triangles\n" << frameCounts.MaterialCount << " materials\n";
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

	InspectorUi::Build(selectedActor);

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

      auto texture = InspectorUi::GetTextureFromCache(reinterpret_cast<uint64>(pCmd->TextureId));
      if (texture)
      {
        renderDevice->SetTexture(0, texture);
      }

			renderDevice->SetPipelineState(_pipelineState);      
			renderDevice->SetSamplerState(0, _samplerState);
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

  InspectorUi::ClearCache();
  InspectorUi::PushTextureToCache(reinterpret_cast<uint64>(&_textureAtlas), _textureAtlas);
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

	_io->Fonts->TexID = &_textureAtlas;
  InspectorUi::PushTextureToCache(reinterpret_cast<uint64>(&_textureAtlas), _textureAtlas);
}

void DebugUi::AddChildNodes(const std::vector<std::shared_ptr<SceneNode>>& childNodes)
{
  for (auto childNode : childNodes)
  {
    if (ImGui::TreeNode(childNode->GetName().c_str()))
    {
      AddChildActors(childNode->GetActors());
      ImGui::TreePop();
    }
  }
}

void DebugUi::AddChildActors(const std::vector<std::shared_ptr<Actor>>& actors)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
  for (uint64 i = 0; i < actors.size(); i++)
  {
    ImGui::TreeNodeEx(actors[i]->GetName().c_str(), flags | (selectedActorIndex == i ? ImGuiTreeNodeFlags_Selected : 0));
		if (ImGui::IsItemClicked())
		{
			selectedActor = actors[i];
			selectedActorIndex = i;
		}
  }
}

void InspectorUi::Build(const std::shared_ptr<Actor>& actor)
{
	if (!actor)
	{
		return;
	}

	ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	ImGui::Begin("Inspector", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

	BuildTransform(actor->GetTransform());
	BuildRenderable(actor->GetComponent<Renderable>());

	ImGui::SetWindowPos(ImVec2(screenSize.x - ImGui::GetWindowWidth(), 0));
	ImGui::End();
}

void InspectorUi::PushTextureToCache(uint64 ptr, const std::shared_ptr<Texture>& texture)
{
  _textureCache[ptr] = texture;
}

std::shared_ptr<Texture> InspectorUi::GetTextureFromCache(uint64 ptr)
{
  return _textureCache[ptr];
}

void InspectorUi::ClearCache()
{
  _textureCache.clear();
}

void InspectorUi::BuildTransform(const std::shared_ptr<Transform>& transform)
{
	ImGui::Separator();
	ImGui::Text("Transform");
	{
		Vector3 position = transform->GetPosition();
		float32 pos[]{ position.X, position.Y, position.Z };
		ImGui::DragFloat3("Position", pos, 2);
		transform->SetPosition(Vector3(pos[0], pos[1], pos[2]));
	}

	{
		Vector3 scale = transform->GetScale();
		float32 scl[]{ scale.X, scale.Y, scale.Z };
		ImGui::DragFloat3("Scale", scl, 2);
		transform->SetScale(Vector3(scl[0], scl[1], scl[2]));
	}

	{
		std::array<Radian, 3> euler = transform->GetRotation().ToEuler();
		float32 angles[3] = { euler[0].InDegrees(), euler[1].InDegrees(), euler[2].InDegrees() };
		ImGui::DragFloat3("Orientation", angles, 1);
		transform->SetRotation(Quaternion(Degree(angles[0]), Degree(angles[1]), Degree(angles[2])));
	}
}

std::unordered_map<uint64, std::shared_ptr<Texture>> InspectorUi::_textureCache;

void InspectorUi::BuildRenderable(const std::shared_ptr<Renderable>& renderable)
{
	if (!renderable)
	{
		return;
	}

	auto material = renderable->GetMesh()->GetMaterial();

	ImGui::Separator();
	ImGui::Text("Renderable");
	{
		Colour ambient = material->GetAmbientColour();
		float32 col[3] = {ambient[0], ambient[1], ambient[2] };
		ImGui::ColorEdit3("Ambient", col);
		material->SetAmbientColour(Colour(col[0] * 255, col[1] * 255, col[2] * 255));
	}
	{
		Colour diffuse = material->GetDiffuseColour();
		float32 col[3] = { diffuse[0], diffuse[1], diffuse[2] };
		ImGui::ColorEdit3("Diffuse", col);
		material->SetDiffuseColour(Colour(col[0] * 255, col[1] * 255, col[2] * 255));
	}
	{
		Colour specular = material->GetSpecularColour();
		float32 col[3] = { specular[0], specular[1], specular[2] };
		ImGui::ColorEdit3("Specular", col);
		material->SetSpecularColour(Colour(col[0] * 255, col[1] * 255, col[2] * 255));
	}
	{
		float32 exponent = material->GetSpecularExponent();
		ImGui::SliderFloat("Exponent", &exponent, 0.0f, 256.0f);
		material->SetSpecularExponent(exponent);
	}
	{
		const char* items[] = { "Diffuse", "Normal", "Specular" };
		static int currentItem = 0;
		ImGui::Combo("Texture", &currentItem, items, 3);	

    switch (currentItem)
    {
      case 0:
      {
        auto diffuseTexture = material->GetDiffuseTexture();
        if (diffuseTexture)
        {
          auto width = diffuseTexture->GetWidth();
          auto height = diffuseTexture->GetHeight();
          auto ratio = width / height;

          PushTextureToCache(reinterpret_cast<uint64>(&diffuseTexture), diffuseTexture);
          ImGui::Image(&diffuseTexture, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.9f, ImGui::GetWindowContentRegionWidth() * 0.9f / ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
        }
        break;
      }
      case 1:
      {
        auto normalTexture = material->GetNormalTexture();
        if (normalTexture)
        {
          auto width = normalTexture->GetWidth();
          auto height = normalTexture->GetHeight();
          auto ratio = width / height;

          PushTextureToCache(reinterpret_cast<uint64>(&normalTexture), normalTexture);
          ImGui::Image(&normalTexture, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.9f, ImGui::GetWindowContentRegionWidth() * 0.9f / ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
        }
        break;
      }
      case 2:
      {
        auto specularTexture = material->GetSpecularTexture();
        if (specularTexture)
        {
          auto width = specularTexture->GetWidth();
          auto height = specularTexture->GetHeight();
          auto ratio = width / height;

          PushTextureToCache(reinterpret_cast<uint64>(&specularTexture), specularTexture);
          ImGui::Image(&specularTexture, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.9f, ImGui::GetWindowContentRegionWidth() * 0.9f / ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
        }
        break;
      }
    }

		//ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
		//if (ImGui::IsItemHovered())
		//{
		//	ImGui::BeginTooltip();
		//	float region_sz = 32.0f;
		//	float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > my_tex_w - region_sz) region_x = my_tex_w - region_sz;
		//	float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > my_tex_h - region_sz) region_y = my_tex_h - region_sz;
		//	float zoom = 4.0f;
		//	ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
		//	ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
		//	ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
		//	ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
		//	ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
		//	ImGui::EndTooltip();
		//}
	}
}
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
#include "../RenderApi/CommandBuffer.hpp"
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
    }
    ImGui::End();
  }

  ImGui::EndFrame();
  ImGui::Render();
  draw(ImGui::GetDrawData());
}

void UiManager::draw(ImDrawData *drawData)
{
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

  // Create constant buffer data
  GpuBufferDesc constBufferDesc;
  constBufferDesc.BufferType = BufferType::Constant;
  constBufferDesc.BufferUsage = BufferUsage::Dynamic;
  constBufferDesc.ByteCount = sizeof(Matrix4);
  _constBuffer = _renderDevice->createGpuBuffer(constBufferDesc);
  _constBuffer->writeData(0, constBufferDesc.ByteCount, &orthProj[0][0], AccessType::WriteOnlyDiscardRange);

  // Ensure vertex buffer is large enough
  if (!_vertBuffer || _vertBuffSize < drawData->TotalVtxCount)
  {
    _vertBuffSize = drawData->TotalVtxCount + 5000;

    VertexBufferDesc vertBufferDesc;
    vertBufferDesc.BufferUsage = BufferUsage::Dynamic;
    vertBufferDesc.VertexCount = _vertBuffSize;
    vertBufferDesc.VertexSizeBytes = sizeof(ImDrawVert);
    _vertBuffer = _renderDevice->createVertexBuffer(vertBufferDesc);
  }

  // Ensure index buffer is large enough
  if (!_idxBuffer || _idxBuffSize < drawData->TotalIdxCount)
  {
    _idxBuffSize = drawData->TotalIdxCount + 10000;

    IndexBufferDesc idxBufferDesc;
    idxBufferDesc.BufferUsage = BufferUsage::Default;
    idxBufferDesc.IndexCount = _idxBuffSize;
    idxBufferDesc.IndexType = sizeof(ImDrawIdx) == 2 ? IndexType::UInt16 : IndexType::UInt32;
    _idxBuffer = _renderDevice->createIndexBuffer(idxBufferDesc);
  }

  // Upload vertex and index data
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

  // Begin command buffer recording
  _uiCommandBuffer->begin(CommandBufferUsage::OneTimeSubmit);

  // Begin render pass to default framebuffer (no clear - preserve existing content)
  _uiCommandBuffer->beginRenderPass(nullptr, false, false, false);

  // Set viewport for UI rendering
  ViewportDesc uiViewport;
  uiViewport.TopLeftX = 0;
  uiViewport.TopLeftY = 0;
  uiViewport.Width = fbWidth;
  uiViewport.Height = fbHeight;
  _uiCommandBuffer->setViewport(uiViewport);

  // Set pipeline state
  _uiCommandBuffer->setPipelineState(_pipelineState);

  // Bind vertex and index buffers
  _uiCommandBuffer->bindVertexBuffer(_vertBuffer);
  _uiCommandBuffer->bindIndexBuffer(_idxBuffer);

  // Render command lists
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

      // Set scissor for this draw command
      ScissorDesc scissor;
      scissor.X = clipRect.x;
      scissor.Y = _renderDevice->getRenderHeight() - clipRect.w;
      scissor.W = clipRect.z - clipRect.x;
      scissor.H = clipRect.w - clipRect.y;
      _uiCommandBuffer->setScissor(scissor);

      // Determine which texture to use
      auto texture = TEXTURE_MAP[reinterpret_cast<uint64>(pCmd->TextureId)];
      auto textureToUse = texture ? texture : _textureAtlas;

      // Reset and rebuild resource set for this draw call
      _uiResourceSet->reset();
      _uiResourceSet->addUniformBuffer(0, _constBuffer);
      _uiResourceSet->addTexture(0, textureToUse);
      _uiResourceSet->addSampler(0, _noMipSamplerState);
      _uiResourceSet->build(_renderDevice);

      // Bind resource set and execute draw call
      _uiCommandBuffer->bindResourceSet(_uiResourceSet, 0);
      _uiCommandBuffer->drawIndexed(pCmd->ElemCount,
                                    1,
                                    static_cast<uint32>(idxOffset + pCmd->IdxOffset),
                                    static_cast<uint32>(vertOffset + pCmd->VtxOffset));
    }
    vertOffset += cmdList->VtxBuffer.Size;
    idxOffset += cmdList->IdxBuffer.Size;
  }

  // End render pass and command buffer
  _uiCommandBuffer->endRenderPass();
  _uiCommandBuffer->end();
  _uiCommandBuffer->execute();
}

void UiManager::initialize(std::shared_ptr<RenderDevice> renderDevice)
{
  if (!_initialized)
  {
    _renderDevice = renderDevice;

    // Create UI command buffer
    _uiCommandBuffer = _renderDevice->createCommandBuffer();

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
    samplerStateDesc.MinFiltering = TextureFilteringMode::Nearest;
    samplerStateDesc.MagFiltering = TextureFilteringMode::Nearest;
    _samplerState = _renderDevice->createSamplerState(samplerStateDesc);
  }
  catch (const std::exception &exception)
  {
    throw std::runtime_error("Unable to initialize UI sampler state. " + std::string(exception.what()));
  }

  SamplerStateDesc desc;
  desc.AddressingMode = AddressingMode{TextureAddressMode::Repeat, TextureAddressMode::Repeat, TextureAddressMode::Repeat};
  desc.MinFiltering = TextureFilteringMode::Nearest;
  desc.MinFiltering = TextureFilteringMode::Nearest;
  try
  {
    _noMipSamplerState = _renderDevice->createSamplerState(desc);
  }
  catch (const std::exception &exception)
  {
    throw std::runtime_error("Unable to initialize UI sampler state. " + std::string(exception.what()));
  }

  // Create UI Resource Set Layout
  _uiResourceSetLayout = _renderDevice->createResourceSetLayout();
  _uiResourceSetLayout->addBinding(0, ResourceType::UNIFORM_BUFFER); // Constants buffer
  _uiResourceSetLayout->addBinding(0, ResourceType::TEXTURE_2D);     // DiffuseMap texture (slot 0)
  _uiResourceSetLayout->addBinding(0, ResourceType::SAMPLER);        // Sampler (slot 0)
  _uiResourceSetLayout->build(_renderDevice);

  // Create UI Resource Set
  _uiResourceSet = _renderDevice->createResourceSet(_uiResourceSetLayout);
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

void UiManager::drawDrawables(const std::vector<DrawableComponent> &drawables)
{
}
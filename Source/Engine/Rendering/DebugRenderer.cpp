#include "DebugRenderer.h"

#include "../RenderApi/BlendState.hpp"
#include "../RenderApi/DepthStencilState.hpp"
#include "../RenderApi/Shader.hpp"
#include "../RenderApi/PipelineState.hpp"
#include "../RenderApi/RenderTarget.hpp"
#include "../RenderApi/RasterizerState.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/SamplerState.hpp"
#include "../RenderApi/ShaderParams.hpp"
#include "../RenderApi/Texture.hpp"
#include "../RenderApi/VertexBuffer.hpp"
#include "../RenderApi/VertexLayout.hpp"
#include "../Utility/String.hpp"
#include "../UI/ImGui/imgui.h"
#include "Camera.h"
#include "Drawable.h"

std::vector<Vector3> AabbCoords = {
    Vector3(-1.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0),
    Vector3(1.0, 1.0, 1.0), Vector3(1.0, -1.0, 1.0),
    Vector3(1.0, -1.0, 1.0), Vector3(-1.0, -1.0, 1.0),
    Vector3(-1.0, -1.0, 1.0), Vector3(-1.0, 1.0, 1.0),

    Vector3(-1.0, 1.0, 1.0), Vector3(-1.0, 1.0, -1.0),
    Vector3(-1.0, 1.0, -1.0), Vector3(1.0, 1.0, -1.0),
    Vector3(1.0, 1.0, -1.0), Vector3(1.0, 1.0, 1.0),

    Vector3(1.0, 1.0, -1.0), Vector3(1.0, -1.0, -1.0),
    Vector3(1.0, -1.0, -1.0), Vector3(1.0, -1.0, 1.0),

    Vector3(-1.0, 1.0, -1.0), Vector3(-1.0, -1.0, -1.0),
    Vector3(-1.0, -1.0, -1.0), Vector3(-1.0, -1.0, 1.0),

    Vector3(-1.0, -1.0, -1.0), Vector3(1.0, -1.0, -1.0)};

DebugRenderer::DebugRenderer() : _debugDisplayType(DebugDisplayType::Disabled)
{
}

void DebugRenderer::onInit(const std::shared_ptr<RenderDevice> &renderDevice)
{
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
    shaderParams->AddParam(ShaderParam("CameraBuffer", ShaderParamType::ConstBuffer, 0));

    RasterizerStateDesc rasterizerStateDesc;
    rasterizerStateDesc.CullMode = CullMode::None;

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->CreateShader(vsDesc);
    pipelineDesc.PS = renderDevice->CreateShader(psDesc);
    pipelineDesc.BlendState = renderDevice->CreateBlendState(BlendStateDesc());
    pipelineDesc.RasterizerState = renderDevice->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
    pipelineDesc.VertexLayout = renderDevice->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _gbufferDebugDrawPso = renderDevice->CreatePipelineState(pipelineDesc);
  }
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

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->CreateShader(vsDesc);
    pipelineDesc.PS = renderDevice->CreateShader(psDesc);
    pipelineDesc.BlendState = renderDevice->CreateBlendState(BlendStateDesc());
    pipelineDesc.RasterizerState = renderDevice->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
    pipelineDesc.VertexLayout = renderDevice->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;

    _depthDebugDrawPso = renderDevice->CreatePipelineState(pipelineDesc);
  }
  {
    ShaderDesc vsDesc;
    vsDesc.EntryPoint = "main";
    vsDesc.ShaderLang = ShaderLang::Glsl;
    vsDesc.ShaderType = ShaderType::Vertex;
    vsDesc.Source = String::LoadFromFile("./Shaders/Basic.vert");

    ShaderDesc psDesc;
    psDesc.EntryPoint = "main";
    psDesc.ShaderLang = ShaderLang::Glsl;
    psDesc.ShaderType = ShaderType::Pixel;
    psDesc.Source = String::LoadFromFile("./Shaders/Empty.frag");

    std::vector<VertexLayoutDesc> vertexLayoutDesc{
        VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3)};

    std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
    shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));

    RasterizerStateDesc rasterizerStateDesc{};
    rasterizerStateDesc.FillMode = FillMode::WireFrame;
    rasterizerStateDesc.CullMode = CullMode::None;

    DepthStencilStateDesc depthStencilStateDesc{};
    depthStencilStateDesc.DepthReadEnabled = false;
    depthStencilStateDesc.DepthWriteEnabled = false;

    BlendStateDesc blendStateDesc{};

    PipelineStateDesc pipelineDesc;
    pipelineDesc.VS = renderDevice->CreateShader(vsDesc);
    pipelineDesc.PS = renderDevice->CreateShader(psDesc);
    pipelineDesc.BlendState = renderDevice->CreateBlendState(blendStateDesc);
    pipelineDesc.RasterizerState = renderDevice->CreateRasterizerState(rasterizerStateDesc);
    pipelineDesc.DepthStencilState = renderDevice->CreateDepthStencilState(depthStencilStateDesc);
    pipelineDesc.VertexLayout = renderDevice->CreateVertexLayout(vertexLayoutDesc);
    pipelineDesc.ShaderParams = shaderParams;
    pipelineDesc.Topology = PrimitiveTopology::LineList;

    _drawAabbPso = renderDevice->CreatePipelineState(pipelineDesc);
  }

  GpuBufferDesc aabbBufferDesc;
  aabbBufferDesc.BufferType = BufferType::Constant;
  aabbBufferDesc.BufferUsage = BufferUsage::Stream;
  aabbBufferDesc.ByteCount = sizeof(ObjectBuffer);
  _aabbBuffer = renderDevice->CreateGpuBuffer(aabbBufferDesc);

  VertexBufferDesc aabbVertexBuffDesc;
  aabbVertexBuffDesc.BufferUsage = BufferUsage::Default;
  aabbVertexBuffDesc.VertexCount = AabbCoords.size();
  aabbVertexBuffDesc.VertexSizeBytes = sizeof(Vector3) * AabbCoords.size();
  _aabbVertexBuffer = renderDevice->CreateVertexBuffer(aabbVertexBuffDesc);
  _aabbVertexBuffer->WriteData(0, sizeof(Vector3) * AabbCoords.size(), AabbCoords.data(), AccessType::WriteOnlyDiscardRange);
}

void DebugRenderer::onDrawDebugUi()
{
  ImGui::Separator();
  {
    ImGui::Text("Debug Renderer");

    std::vector<const char *> debugRenderingItems = {"Disabled", "Diffuse", "Normal", "Depth"};
    static int debugRenderingCurrentItem = 0;
    ImGui::Combo("Target", &debugRenderingCurrentItem, debugRenderingItems.data(), debugRenderingItems.size());
    _debugDisplayType = static_cast<DebugDisplayType>(debugRenderingCurrentItem);
  }
}

void DebugRenderer::drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                              const std::shared_ptr<RenderTarget> &gBuffer,
                              const std::shared_ptr<RenderTarget> &lightingBuffer,
                              const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                              const Camera &camera)
{
  switch (_debugDisplayType)
  {
  case DebugDisplayType::Disabled:
  {
    drawRenderTarget(renderDevice, lightingBuffer->GetColourTarget(0), camera);
    break;
  }
  case DebugDisplayType::Diffuse:
  {
    drawRenderTarget(renderDevice, gBuffer->GetColourTarget(0), camera);
    break;
  }
  case DebugDisplayType::Normal:
  {
    drawRenderTarget(renderDevice, gBuffer->GetColourTarget(1), camera);
    break;
  }
  case DebugDisplayType::Depth:
  {
    drawRenderTarget(renderDevice, gBuffer->GetDepthStencilTarget(), camera);
    break;
  }
  }

  drawAabb(renderDevice, aabbDrawables, camera);
}

void DebugRenderer::drawAabb(const std::shared_ptr<RenderDevice> &renderDevice,
                             const std::vector<std::shared_ptr<Drawable>> &aabbDrawables,
                             const Camera &camera)
{
  renderDevice->SetPipelineState(_drawAabbPso);
  for (auto drawable : aabbDrawables)
  {
    auto aabb = drawable->getAabb();

    ObjectBuffer objectBufferData;
    objectBufferData.Model = Matrix4::Translation(drawable->getPosition()) * Matrix4::Scaling(aabb.GetHalfSize());
    objectBufferData.ModelView = camera.getView() * objectBufferData.Model;
    objectBufferData.ModelViewProjection = camera.getProj() * objectBufferData.ModelView;
    _aabbBuffer->WriteData(0, sizeof(ObjectBuffer), &objectBufferData, AccessType::WriteOnlyDiscard);

    renderDevice->SetConstantBuffer(0, _aabbBuffer);
    renderDevice->SetVertexBuffer(_aabbVertexBuffer);
    renderDevice->Draw(AabbCoords.size(), 0);
  }
}

void DebugRenderer::drawRenderTarget(std::shared_ptr<RenderDevice> renderDevice,
                                     std::shared_ptr<Texture> renderTarget,
                                     const Camera &camera)
{
  CameraBufferData cameraBufferData;
  cameraBufferData.FarClip = camera.getFar();
  cameraBufferData.NearClip = camera.getNear();
  _cameraBuffer->WriteData(0, sizeof(CameraBufferData), &cameraBufferData, AccessType::WriteOnlyDiscard);

  if (renderTarget->GetDesc().Usage == TextureUsage::RenderTarget)
  {
    renderDevice->SetPipelineState(_gbufferDebugDrawPso);
  }
  else if (renderTarget->GetDesc().Usage == TextureUsage::Depth)
  {
    renderDevice->SetConstantBuffer(0, _cameraBuffer);
    renderDevice->SetPipelineState(_depthDebugDrawPso);
  }
  else
  {
    return;
  }

  renderDevice->SetRenderTarget(nullptr);
  renderDevice->ClearBuffers(RTT_Colour | RTT_Depth);
  renderDevice->SetTexture(0, renderTarget);
  renderDevice->SetSamplerState(0, _noMipSamplerState);
  renderDevice->SetVertexBuffer(_fsQuadBuffer);
  renderDevice->Draw(6, 0);
}
#include "Renderer.h"

#include "../Maths/Matrix4.hpp"
#include "../Utility/Assert.hpp"
#include "../Utility/String.hpp"
#include "../RenderApi/GL/GLRenderDevice.hpp"
#include "../RenderApi/PipelineState.hpp"
#include "../RenderApi/ShaderParams.hpp"
#include "../SceneManagement/Camera.hpp"
#include "../SceneManagement/Transform.h"
#include "Renderable.hpp"
#include "StaticMesh.h"

using namespace Rendering;

struct ConstBufferData
{
  Matrix4 Proj;
  Matrix4 View;
};

Renderer::Renderer(const RendererDesc& desc) : _desc(desc)
{
  try
  {
    Assert::ThrowIfFalse(desc.RenderApi == RenderApi::GL40, "Only OpenGL 4.0 is supported");
    
    RenderDeviceDesc renderDeviceDesc;
    renderDeviceDesc.RenderWidth = _desc.RenderWidth;
    renderDeviceDesc.RenderHeight = _desc.RenderHeight;
    _renderDevice.reset(new GLRenderDevice(renderDeviceDesc));
    
    InitPipelineStates();
    InitConstBuffer();
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
  for (auto& renderable : _renderables)
  {
    for (uint32 i = 0; i < renderable.Renderable->GetMeshCount(); i++)
    {
      auto mesh = renderable.Renderable->GetMeshAtIndex(i);
      _renderDevice->SetVertexBuffer(0, mesh->GetVertexData(_renderDevice));
      if (mesh->IsIndexed())
      {
        _renderDevice->SetIndexBuffer(mesh->GetIndexData(_renderDevice));
        _renderDevice->DrawIndexed(mesh->GetIndexCount(), 0, 0);
      }
      else
      {
        _renderDevice->Draw(mesh->GetVertexCount(), 0);
      }
    }
  }
}

void Renderer::InitPipelineStates()
{
  ShaderDesc vsDesc;
  vsDesc.EntryPoint = "main";
  vsDesc.ShaderLang = ShaderLang::Glsl;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::LoadFromFile("./../../Resources/Shaders/BasicVS.glsl");
  
  ShaderDesc psDesc;
  psDesc.EntryPoint = "main";
  psDesc.ShaderLang = ShaderLang::Glsl;
  psDesc.ShaderType = ShaderType::Pixel;
  psDesc.Source = String::LoadFromFile("./../../Resources/Shaders/BasicPS.glsl");
  
  std::vector<VertexLayoutDesc> vertexLayoutDesc
  {
    VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3, 0)
  };
  
  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->AddParam(ShaderParam("Constants", ShaderParamType::ConstBuffer, 0));
  
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
    
    _basicPipeline = _renderDevice->CreatePipelineState(pipelineDesc);
  }
  catch (const std::exception& exception)
  {
    throw std::runtime_error("Unable to initialize pipeline states. " + std::string(exception.what()));
  }
}

void Renderer::InitConstBuffer()
{
  try
  {
    GpuBufferDesc perShaderBuffDesc;
    perShaderBuffDesc.BufferType = BufferType::Constant;
    perShaderBuffDesc.BufferUsage = BufferUsage::Dynamic;
    perShaderBuffDesc.ByteCount = sizeof(ConstBufferData);
    _constBuffer = _renderDevice->CreateGpuBuffer(perShaderBuffDesc);
  }
  catch (const std::exception& exception)
  {
    throw std::runtime_error(std::string("Could not initialize constant buffer\n") + exception.what());
  }
}

void Renderer::StartFrame()
{
  ConstBufferData data;
  data.Proj = _activeCamera->GetProjection();
  data.View = _activeCamera->GetView();
  _constBuffer->WriteData(0, sizeof(ConstBufferData), &data);
  
  _renderDevice->SetPipelineState(_basicPipeline);
}

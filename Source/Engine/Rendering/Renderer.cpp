#include "Renderer.h"

#include <iostream>

#include "../Core/Maths.h"
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

Renderer::Renderer() : _settingsModified(false)
{
}

bool Renderer::init(const std::shared_ptr<RenderDevice> &renderDevice)
{
  try
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
      psDesc.Source = String::LoadFromFile("./Shaders/TexturedQuad.frag");

      std::vector<VertexLayoutDesc> vertexLayoutDesc{
          VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
          VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
      };

      std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
      shaderParams->AddParam(ShaderParam("TextureMap", ShaderParamType::Texture, 0));

      DepthStencilStateDesc depthStencilStateDesc{};
      depthStencilStateDesc.DepthReadEnabled = false;
      depthStencilStateDesc.DepthWriteEnabled = false;

      PipelineStateDesc pipelineDesc;
      pipelineDesc.VS = renderDevice->CreateShader(vsDesc);
      pipelineDesc.PS = renderDevice->CreateShader(psDesc);
      pipelineDesc.BlendState = renderDevice->CreateBlendState(BlendStateDesc{});
      pipelineDesc.RasterizerState = renderDevice->CreateRasterizerState(RasterizerStateDesc{});
      pipelineDesc.DepthStencilState = renderDevice->CreateDepthStencilState(depthStencilStateDesc);
      pipelineDesc.VertexLayout = renderDevice->CreateVertexLayout(vertexLayoutDesc);
      pipelineDesc.ShaderParams = shaderParams;

      _downsamplePso = renderDevice->CreatePipelineState(pipelineDesc);
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
      psDesc.Source = String::LoadFromFile("./Shaders/VerticalBlur.frag");

      std::vector<VertexLayoutDesc> vertexLayoutDesc{
          VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float2),
          VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
      };

      std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
      shaderParams->AddParam(ShaderParam("TextureMap", ShaderParamType::Texture, 0));

      DepthStencilStateDesc depthStencilStateDesc{};
      depthStencilStateDesc.DepthReadEnabled = false;
      depthStencilStateDesc.DepthWriteEnabled = false;

      PipelineStateDesc pipelineDesc;
      pipelineDesc.VS = renderDevice->CreateShader(vsDesc);
      pipelineDesc.PS = renderDevice->CreateShader(psDesc);
      pipelineDesc.BlendState = renderDevice->CreateBlendState(BlendStateDesc{});
      pipelineDesc.RasterizerState = renderDevice->CreateRasterizerState(RasterizerStateDesc{});
      pipelineDesc.DepthStencilState = renderDevice->CreateDepthStencilState(depthStencilStateDesc);
      pipelineDesc.VertexLayout = renderDevice->CreateVertexLayout(vertexLayoutDesc);
      pipelineDesc.ShaderParams = shaderParams;

      _verticalBlurPso = renderDevice->CreatePipelineState(pipelineDesc);

      psDesc.Source = String::LoadFromFile("./Shaders/HorizontalBlur.frag");
      _horizontalBlurPso = renderDevice->CreatePipelineState(pipelineDesc);
    }

    VertexBufferDesc vtxBuffDesc;
    vtxBuffDesc.BufferUsage = BufferUsage::Default;
    vtxBuffDesc.VertexCount = FullscreenQuadVertices.size();
    vtxBuffDesc.VertexSizeBytes = sizeof(FullscreenQuadVertex);
    _fsQuadBuffer = renderDevice->CreateVertexBuffer(vtxBuffDesc);
    _fsQuadBuffer->WriteData(0, sizeof(FullscreenQuadVertex) * FullscreenQuadVertices.size(), FullscreenQuadVertices.data(), AccessType::WriteOnlyDiscardRange);

    SamplerStateDesc basicSamplerStateDesc;
    basicSamplerStateDesc.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
    basicSamplerStateDesc.MinFiltering = TextureFilteringMode::Linear;
    basicSamplerStateDesc.MinFiltering = TextureFilteringMode::Linear;
    basicSamplerStateDesc.MipFiltering = TextureFilteringMode::Linear;
    _basicSamplerState = renderDevice->CreateSamplerState(basicSamplerStateDesc);

    SamplerStateDesc noMipSamplerState;
    noMipSamplerState.AddressingMode = AddressingMode{TextureAddressMode::Wrap, TextureAddressMode::Wrap, TextureAddressMode::Wrap};
    noMipSamplerState.MinFiltering = TextureFilteringMode::None;
    noMipSamplerState.MinFiltering = TextureFilteringMode::None;
    noMipSamplerState.MipFiltering = TextureFilteringMode::None;
    _noMipSamplerState = renderDevice->CreateSamplerState(noMipSamplerState);

    GpuBufferDesc cameraBufferDesc;
    cameraBufferDesc.BufferType = BufferType::Constant;
    cameraBufferDesc.BufferUsage = BufferUsage::Stream;
    cameraBufferDesc.ByteCount = sizeof(CameraBufferData);
    _cameraBuffer = renderDevice->CreateGpuBuffer(cameraBufferDesc);

    GpuBufferDesc blurBufferDesc;
    blurBufferDesc.BufferType = BufferType::Constant;
    blurBufferDesc.BufferUsage = BufferUsage::Stream;
    blurBufferDesc.ByteCount = sizeof(CameraBufferData);
    _blurBuffer = renderDevice->CreateGpuBuffer(blurBufferDesc);

    onInit(renderDevice);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Failed to initialize Renderer. " << e.what() << std::endl;
    return false;
  }
  return true;
}

void Renderer::drawDebugUi()
{
  onDrawDebugUi();
}
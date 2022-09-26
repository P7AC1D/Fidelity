#include "Renderer.h"

#include <iostream>

#include "../Core/Maths.h"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/VertexBuffer.hpp"
#include "../RenderApi/VertexLayout.hpp"

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
{}

bool Renderer::init(const std::shared_ptr<RenderDevice> &renderDevice)
{
  try
  {
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
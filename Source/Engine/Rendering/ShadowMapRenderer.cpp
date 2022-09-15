#include "ShadowMapRenderer.h"

#include "../RenderApi/Texture.hpp"
#include "../RenderApi/PipelineState.hpp"
#include "../RenderApi/RenderTarget.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/SamplerState.hpp"
#include "../Utility/String.hpp"
#include "../UI/ImGui/imgui.h"
#include "Camera.h"
#include "Drawable.h"
#include "Light.h"
#include "StaticMesh.h"

#include <algorithm>
#include <cmath>

struct LightDepthData
{
  Matrix4 LightTransform;
};

Matrix4 calculateLightProjFromFustrum(const Matrix4 &lightView, const std::shared_ptr<Camera> &camera, float32 zMulti)
{
  Matrix4 projView(camera->getProj() * camera->getView());
  Matrix4 projViewInvs(projView.Inverse());

  std::vector<Vector4> frustrumCorners;
  for (uint32 x = 0; x < 2; ++x)
  {
    for (uint32 y = 0; y < 2; ++y)
    {
      for (uint32 z = 0; z < 2; ++z)
      {
        const Vector4 point = projViewInvs * Vector4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
        frustrumCorners.push_back(point);
      }
    }
  }

  Vector4 max(std::numeric_limits<float32>::min()), min(std::numeric_limits<float32>::max());
  for (const Vector4 &corner : frustrumCorners)
  {
    const Vector4 transformedPoint = lightView * corner;
    min = Math::Min(transformedPoint, min);
    max = Math::Max(transformedPoint, max);
  }

  if (min.Z < 0)
  {
    min.Z *= zMulti;
  }
  else
  {
    min.Z /= zMulti;
  }

  if (max.Z < 0)
  {
    max.Z /= zMulti;
  }
  else
  {
    max.Z *= zMulti;
  }

  return Matrix4::Orthographic(min.X, max.X, min.Y, max.Y, min.Z, max.Z);
}

ShadowMapRenderer::ShadowMapRenderer() : _zMulti(10.0),
                                         _shadowMapResolution(1024),
                                         _cascadeCount(4)
{
}

void ShadowMapRenderer::onInit(const std::shared_ptr<RenderDevice> &renderDevice)
{
  TextureDesc shadowMapDesc;
  shadowMapDesc.Width = _shadowMapResolution;
  shadowMapDesc.Height = _shadowMapResolution;
  shadowMapDesc.Usage = TextureUsage::Depth;
  shadowMapDesc.Type = TextureType::Texture2D;
  shadowMapDesc.Format = TextureFormat::D32F;

  RenderTargetDesc rtDesc;
  rtDesc.DepthStencilTarget = renderDevice->CreateTexture(shadowMapDesc);
  rtDesc.Height = _shadowMapResolution;
  rtDesc.Width = _shadowMapResolution;

  _shadowMapRto = renderDevice->CreateRenderTarget(rtDesc);

  ShaderDesc vsDesc;
  vsDesc.EntryPoint = "main";
  vsDesc.ShaderLang = ShaderLang::Glsl;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::LoadFromFile("./Shaders/ShadowMap.vert");

  ShaderDesc psDesc;
  psDesc.EntryPoint = "main";
  psDesc.ShaderLang = ShaderLang::Glsl;
  psDesc.ShaderType = ShaderType::Pixel;
  psDesc.Source = String::LoadFromFile("./Shaders/Empty.frag");

  std::vector<VertexLayoutDesc> vertexLayoutDesc{
      VertexLayoutDesc(SemanticType::Position, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Normal, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::TexCoord, SemanticFormat::Float2),
      VertexLayoutDesc(SemanticType::Tangent, SemanticFormat::Float3),
      VertexLayoutDesc(SemanticType::Bitangent, SemanticFormat::Float3)};

  std::shared_ptr<ShaderParams> shaderParams(new ShaderParams());
  shaderParams->AddParam(ShaderParam("ObjectBuffer", ShaderParamType::ConstBuffer, 0));
  shaderParams->AddParam(ShaderParam("TransformsBuffer", ShaderParamType::ConstBuffer, 1));

  RasterizerStateDesc rasterizerStateDesc;
  rasterizerStateDesc.CullMode = CullMode::Clockwise;

  PipelineStateDesc pipelineDesc;
  pipelineDesc.VS = renderDevice->CreateShader(vsDesc);
  pipelineDesc.PS = renderDevice->CreateShader(psDesc);
  pipelineDesc.BlendState = renderDevice->CreateBlendState(BlendStateDesc{});
  pipelineDesc.RasterizerState = renderDevice->CreateRasterizerState(RasterizerStateDesc{});
  pipelineDesc.DepthStencilState = renderDevice->CreateDepthStencilState(DepthStencilStateDesc());
  pipelineDesc.VertexLayout = renderDevice->CreateVertexLayout(vertexLayoutDesc);
  pipelineDesc.ShaderParams = shaderParams;

  _shadowMapPso = renderDevice->CreatePipelineState(pipelineDesc);

  GpuBufferDesc objectBufferDesc;
  objectBufferDesc.BufferType = BufferType::Constant;
  objectBufferDesc.BufferUsage = BufferUsage::Stream;
  objectBufferDesc.ByteCount = sizeof(ObjectBuffer);
  _objectBuffer = renderDevice->CreateGpuBuffer(objectBufferDesc);

  GpuBufferDesc transformsBufferDesc;
  transformsBufferDesc.BufferType = BufferType::Constant;
  transformsBufferDesc.BufferUsage = BufferUsage::Stream;
  transformsBufferDesc.ByteCount = sizeof(LightDepthData);
  _transformsBuffer = renderDevice->CreateGpuBuffer(transformsBufferDesc);

  GpuBufferDesc csmBufferDesc;
  csmBufferDesc.BufferType = BufferType::Constant;
  csmBufferDesc.BufferUsage = BufferUsage::Stream;
  csmBufferDesc.ByteCount = sizeof(ShadowMapData);
  _shadowBufer = renderDevice->CreateGpuBuffer(csmBufferDesc);
}

void ShadowMapRenderer::onDrawDebugUi()
{
  ImGui::Separator();
  {
    ImGui::Text("Shadow Map Renderer");

    float32 zMultiple = _zMulti;
    if (ImGui::DragFloat("Z-Multiple", &zMultiple, 0.1f, 0.1f, 1000.0f))
    {
      _zMulti = zMultiple;
    }

    int shadowMapResolution = _shadowMapResolution;
    if (ImGui::SliderInt("Resolution", &shadowMapResolution, 256, 8192))
    {
      _shadowMapResolution = shadowMapResolution;
      _settingsModified = true;
    }
  }
}

void ShadowMapRenderer::drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                                  const std::vector<std::shared_ptr<Drawable>> &drawables,
                                  const std::vector<std::shared_ptr<Light>> &lights,
                                  const std::shared_ptr<Camera> &camera)
{
  if (_settingsModified)
  {
    TextureDesc shadowMapDesc;
    shadowMapDesc.Width = _shadowMapResolution;
    shadowMapDesc.Height = _shadowMapResolution;
    shadowMapDesc.Usage = TextureUsage::Depth;
    shadowMapDesc.Type = TextureType::Texture2D;
    shadowMapDesc.Format = TextureFormat::D32F;

    RenderTargetDesc rtDesc;
    rtDesc.DepthStencilTarget = renderDevice->CreateTexture(shadowMapDesc);
    rtDesc.Height = _shadowMapResolution;
    rtDesc.Width = _shadowMapResolution;

    _shadowMapRto = renderDevice->CreateRenderTarget(rtDesc);
    _settingsModified = false;
  }

  // TODO: Should either support one direction light or multiple
  std::shared_ptr<Light> directionalLight;
  for (auto light : lights)
  {
    if (light->getLightType() == LightType::Directional)
    {
      directionalLight = light;
    }
  }

  renderDevice->SetPipelineState(_shadowMapPso);

  ViewportDesc viewportDesc;
  viewportDesc.Height = _shadowMapResolution;
  viewportDesc.Width = _shadowMapResolution;
  renderDevice->SetViewport(viewportDesc);

  renderDevice->SetRenderTarget(_shadowMapRto);
  renderDevice->ClearBuffers(RTT_Depth);

  Matrix4 lightView(Matrix4::LookAt(-directionalLight->getDirection(), Vector3::Zero, Vector3(0.0f, 1.0f, 0.0f)));
  Matrix4 lightProj(calculateLightProjFromFustrum(lightView, camera, _zMulti));

  LightDepthData lightDepthData;
  lightDepthData.LightTransform = lightProj * lightView;
  _transformsBuffer->WriteData(0, sizeof(LightDepthData), &lightDepthData, AccessType::WriteOnlyDiscard);

  ShadowMapData shadowMapData;
  shadowMapData.CameraFarPlane = camera->getFar();
  shadowMapData.LightDirection = directionalLight->getDirection();
  shadowMapData.LightTransform = lightDepthData.LightTransform;
  _shadowBufer->WriteData(0, sizeof(ShadowMapData), &shadowMapData, AccessType::WriteOnlyDiscard);

  renderDevice->SetConstantBuffer(1, _transformsBuffer);
  for (const auto &drawable : drawables)
  {
    writeObjectConstantData(drawable, camera);

    renderDevice->SetConstantBuffer(0, _objectBuffer);

    auto mesh = drawable->getMesh();
    renderDevice->SetVertexBuffer(mesh->getVertexData(renderDevice));

    if (mesh->isIndexed())
    {
      auto indexCount = mesh->getIndexCount();
      renderDevice->SetIndexBuffer(mesh->getIndexData(renderDevice));
      renderDevice->DrawIndexed(indexCount, 0, 0);
    }
    else
    {
      renderDevice->Draw(mesh->getVertexCount(), 0);
    }
  }
}

void ShadowMapRenderer::writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const
{
  ObjectBuffer objectBufferData;
  objectBufferData.Model = drawable->getMatrix();
  objectBufferData.ModelView = camera->getView() * objectBufferData.Model;
  objectBufferData.ModelViewProjection = camera->getProj() * objectBufferData.ModelView;
  _objectBuffer->WriteData(0, sizeof(ObjectBuffer), &objectBufferData, AccessType::WriteOnlyDiscard);
}
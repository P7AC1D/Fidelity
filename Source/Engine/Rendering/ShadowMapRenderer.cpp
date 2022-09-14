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

ShadowMapRenderer::ShadowMapRenderer() : _zMulti(100.0),
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
  shadowMapDesc.Type = TextureType::Texture2DArray;
  shadowMapDesc.Format = TextureFormat::D32F;
  shadowMapDesc.Count = 4;

  RenderTargetDesc rtDesc;
  rtDesc.DepthStencilTarget = renderDevice->CreateTexture(shadowMapDesc);
  rtDesc.Height = _shadowMapResolution;
  rtDesc.Width = _shadowMapResolution;

  _shadowMapRto = renderDevice->CreateRenderTarget(rtDesc);

  _shadowMapSamplerState = renderDevice->CreateSamplerState(SamplerStateDesc{});

  ShaderDesc vsDesc;
  vsDesc.EntryPoint = "main";
  vsDesc.ShaderLang = ShaderLang::Glsl;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::LoadFromFile("./Shaders/CascadeShadowMap.vert");

  ShaderDesc gsDesc;
  gsDesc.EntryPoint = "main";
  gsDesc.ShaderLang = ShaderLang::Glsl;
  gsDesc.ShaderType = ShaderType::Geometry;
  gsDesc.Source = String::LoadFromFile("./Shaders/CascadeShadowMap.geom");

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

  PipelineStateDesc pipelineDesc;
  pipelineDesc.VS = renderDevice->CreateShader(vsDesc);
  pipelineDesc.GS = renderDevice->CreateShader(gsDesc);
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
  transformsBufferDesc.ByteCount = sizeof(Matrix4) * 4;
  _transformsBuffer = renderDevice->CreateGpuBuffer(transformsBufferDesc);

  GpuBufferDesc csmBufferDesc;
  csmBufferDesc.BufferType = BufferType::Constant;
  csmBufferDesc.BufferUsage = BufferUsage::Stream;
  csmBufferDesc.ByteCount = sizeof(CascadeShadowMapData);
  _cascadeShadowBuffer = renderDevice->CreateGpuBuffer(csmBufferDesc);
}

void ShadowMapRenderer::onDrawDebugUi()
{
  ImGui::Separator();
  {
    ImGui::Text("Shadow Map Renderer");
    ImGui::DragFloat("Z-Multiple", &_zMulti, 0.1f, 0.1f, 1000.0f);
    if (ImGui::SliderInt("Resolution", &_shadowMapResolution, 256, 8192))
    {
      _settingsModified = true;
    }
  }
}

void ShadowMapRenderer::drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                                  const std::vector<std::shared_ptr<Drawable>> &drawables,
                                  const std::vector<std::shared_ptr<Light>> &lights,
                                  const Camera &camera)
{
  if (_settingsModified)
  {
    TextureDesc shadowMapDesc;
    shadowMapDesc.Width = _shadowMapResolution;
    shadowMapDesc.Height = _shadowMapResolution;
    shadowMapDesc.Usage = TextureUsage::Depth;
    shadowMapDesc.Type = TextureType::Texture2DArray;
    shadowMapDesc.Format = TextureFormat::D32F;
    shadowMapDesc.Count = 4;

    RenderTargetDesc rtDesc;
    rtDesc.DepthStencilTarget = renderDevice->CreateTexture(shadowMapDesc);
    rtDesc.Height = _shadowMapResolution;
    rtDesc.Width = _shadowMapResolution;

    _shadowMapRto = renderDevice->CreateRenderTarget(rtDesc);
    _settingsModified = false;
  }

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

  auto transforms = createLightTransforms(directionalLight, camera);

  CascadeShadowMapData csmData;
  csmData.CascadeCount = 4;
  csmData.LightDirection = directionalLight->getDirection();
  csmData.LightTransforms[0] = transforms[0];
  csmData.LightTransforms[1] = transforms[1];
  csmData.LightTransforms[2] = transforms[2];
  csmData.LightTransforms[3] = transforms[3];

  auto cascadeLevels = createCascadeLevels(camera);
  csmData.CascadePlaneDistances[0] = cascadeLevels[0];
  csmData.CascadePlaneDistances[1] = cascadeLevels[1];
  csmData.CascadePlaneDistances[2] = cascadeLevels[2];
  csmData.CascadePlaneDistances[3] = cascadeLevels[3];
  _cascadeShadowBuffer->WriteData(0, sizeof(CascadeShadowMapData), &csmData, AccessType::WriteOnlyDiscard);

  _transformsBuffer->WriteData(0, sizeof(Matrix4) * 4, transforms.data(), AccessType::WriteOnlyDiscard);

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

std::vector<Vector4> ShadowMapRenderer::getFrustrumCorners(const Matrix4 &proj, const Matrix4 &cameraView) const
{
  Matrix4 projView = proj * cameraView;
  Matrix4 projViewInvs = projView.Inverse();

  std::vector<Vector4> frustrumCorners;
  for (uint32 x = 0; x < 2; ++x)
  {
    for (uint32 y = 0; y < 2; ++y)
    {
      for (uint32 z = 0; z < 2; ++z)
      {
        const Vector4 point = projViewInvs * Vector4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
        frustrumCorners.push_back(point);
        // frustrumCorners.push_back(point / point.W);
      }
    }
  }
  return frustrumCorners;
}

Matrix4 ShadowMapRenderer::calcLightViewProj(const float32 nearPlane,
                                             const float32 farPlane,
                                             const Camera &camera,
                                             const std::shared_ptr<Light> &directionalLight) const
{
  Matrix4 proj = Matrix4::Perspective(camera.getFov(), camera.getAspectRatio(), nearPlane, farPlane);
  auto frustrumCorners = getFrustrumCorners(proj, camera.getView());

  Matrix4 lightView = calcLightView(frustrumCorners, directionalLight);
  Matrix4 lightProj = calcLightProj(frustrumCorners, lightView);
  return lightProj * lightView;
}

Matrix4 ShadowMapRenderer::calcLightView(const std::vector<Vector4> &frustrumCorners, const std::shared_ptr<Light> &directionalLight) const
{
  Vector3 center(0, 0, 0);
  for (const Vector4 &corner : frustrumCorners)
  {
    center += Vector3(corner);
  }
  center /= frustrumCorners.size();

  Matrix4 lightTransform = Matrix4::LookAt(center + directionalLight->getDirection(), center, Vector3(0.0f, 1.0f, 0.0f));
  return lightTransform;
}

Matrix4 ShadowMapRenderer::calcLightProj(const std::vector<Vector4> &frustrumCorners, const Matrix4 &lightView) const
{
  Vector4 max(std::numeric_limits<float32>::min()), min(std::numeric_limits<float32>::max());
  for (const Vector4 &corner : frustrumCorners)
  {
    const Vector4 transformedPoint = lightView * corner;
    min = Math::Min(transformedPoint, min);
    max = Math::Max(transformedPoint, max);
  }

  if (min.Z < 0)
  {
    min.Z *= _zMulti;
  }
  else
  {
    min.Z /= _zMulti;
  }

  if (max.Z < 0)
  {
    max.Z /= _zMulti;
  }
  else
  {
    max.Z *= _zMulti;
  }

  return Matrix4::Orthographic(min.X, max.X, min.Y, max.Y, min.Z, max.Z);
}

void ShadowMapRenderer::writeObjectConstantData(std::shared_ptr<Drawable> drawable, const Camera &camera) const
{
  ObjectBuffer objectBufferData;
  objectBufferData.Model = drawable->getMatrix();
  objectBufferData.ModelView = camera.getView() * objectBufferData.Model;
  objectBufferData.ModelViewProjection = camera.getProj() * objectBufferData.ModelView;
  _objectBuffer->WriteData(0, sizeof(ObjectBuffer), &objectBufferData, AccessType::WriteOnlyDiscard);
}

std::vector<Matrix4> ShadowMapRenderer::createLightTransforms(const std::shared_ptr<Light> &directionalLight,
                                                              const Camera &camera) const
{
  float32 cameraFarPlane = camera.getFar();
  float32 cameraNearPlane = camera.getNear();
  std::vector<float32> shadowCascadeLevels = createCascadeLevels(camera);

  std::vector<Matrix4>
      transforms;
  for (uint64 i = 0; i < shadowCascadeLevels.size() + 1; ++i)
  {
    // transforms.push_back(calcLightViewProj(cameraNearPlane, cameraFarPlane, camera, directionalLight));
    // continue;

    if (i == 0)
    {
      transforms.push_back(calcLightViewProj(cameraNearPlane, shadowCascadeLevels[i], camera, directionalLight));
    }
    else if (i < shadowCascadeLevels.size())
    {
      transforms.push_back(calcLightViewProj(shadowCascadeLevels[i - 1], shadowCascadeLevels[i], camera, directionalLight));
    }
    else
    {
      transforms.push_back(calcLightViewProj(shadowCascadeLevels[i - 1], cameraFarPlane, camera, directionalLight));
    }
  }
  return transforms;
}

std::vector<float32> ShadowMapRenderer::createCascadeLevels(const Camera &camera) const
{
  float32 cameraFarPlane = camera.getFar();
  float32 cameraNearPlane = camera.getNear();
  return std::vector<float32>{cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f};
}
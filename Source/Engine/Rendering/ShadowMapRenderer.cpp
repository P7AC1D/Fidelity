#include "ShadowMapRenderer.h"

#include "../RenderApi/Texture.hpp"
#include "../RenderApi/PipelineState.hpp"
#include "../RenderApi/RenderTarget.hpp"
#include "../RenderApi/RenderDevice.hpp"
#include "../RenderApi/SamplerState.hpp"
#include "../Utility/String.hpp"
#include "../Utility/Assert.hpp"
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

float32 calculateCascadeRadius(const std::vector<Vector3> &frustrumCorners, const Vector3 &frustrumCenter)
{
  Assert::ThrowIfFalse(frustrumCorners.size() == 8, "Invalid size of supplied frustrum corners.");

  float32 sphereRadius = 0.0f;
  for (uint32 i = 0; i < 8; i++)
  {
    float32 dist = (frustrumCorners[i] - frustrumCenter).Length();
    sphereRadius = std::max(sphereRadius, dist);
  }

  sphereRadius = std::ceil(sphereRadius * 16.0f) / 16.0f;
  return sphereRadius;
}

Vector3 calculateFrustrumCenter(const std::vector<Vector3> &frustrumCorners)
{
  Vector3 center(Vector3::Zero);
  for (uint32 i = 0; i < 8; ++i)
  {
    center = center + frustrumCorners[i];
  }
  return center * (1.0f / 8.0f);
}

std::vector<Vector3> calculateFrustrumCorners(const Matrix4 &view, const Matrix4 &projection)
{
  std::vector<Vector4> frustrumCornersVS = {
      Vector4(-1.0f, 1.0f, 0.0f, 1.0f),
      Vector4(1.0f, 1.0f, 0.0f, 1.0f),
      Vector4(1.0f, -1.0f, 0.0f, 1.0f),
      Vector4(-1.0f, -1.0f, 0.0f, 1.0f),
      Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
      Vector4(1.0f, 1.0f, 1.0f, 1.0f),
      Vector4(1.0f, -1.0f, 1.0f, 1.0f),
      Vector4(-1.0f, -1.0f, 1.0f, 1.0f)};

  Matrix4 projView(projection * view);
  Matrix4 projViewInvs(projView.Inverse());

  std::vector<Vector3> frustrumCornersWS;
  for (uint32 i = 0; i < 8; i++)
  {
    Vector4 point = projViewInvs * frustrumCornersVS[i];
    point = point / point.W;

    frustrumCornersWS.push_back(Vector3(point.X, point.Y, point.Z));
  }

  return frustrumCornersWS;
}

ShadowMapRenderer::ShadowMapRenderer() : _zMulti(3.0),
                                         _shadowMapResolution(2048),
                                         _cascadeCount(4),
                                         _drawCascadeLayers(false),
                                         _sampleCount(16.0f),
                                         _sampleSpread(700.0f),
                                         _minCascadeDistance(0.0f),
                                         _maxCascadeDistance(1.0f),
                                         _cascadeLambda(0.4f)
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
  shadowMapDesc.Count = _cascadeCount;

  RenderTargetDesc rtDesc;
  rtDesc.DepthStencilTarget = renderDevice->CreateTexture(shadowMapDesc);
  rtDesc.Height = _shadowMapResolution;
  rtDesc.Width = _shadowMapResolution;

  _shadowMapRto = renderDevice->CreateRenderTarget(rtDesc);

  ShaderDesc vsDesc;
  vsDesc.EntryPoint = "main";
  vsDesc.ShaderLang = ShaderLang::Glsl;
  vsDesc.ShaderType = ShaderType::Vertex;
  vsDesc.Source = String::LoadFromFile("./Shaders/CascadeShadowMap.vert");

  // TODO: Change to only use Pixel shaders as it allows for a dynamic cascade count - apparently doesn't affect performance that much
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

  RasterizerStateDesc rasterizerStateDesc;
  rasterizerStateDesc.CullMode = CullMode::Clockwise;

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

    int sampleCount = _sampleCount;
    if (ImGui::SliderInt("Sample Count", &sampleCount, 1, 64))
    {
      _sampleCount = sampleCount;
    }

    float32 sampleSpread = _sampleSpread;
    if (ImGui::SliderFloat("Sample Spread", &sampleSpread, 1.0f, 1000.0f))
    {
      _sampleSpread = sampleSpread;
    }

    float32 cascadeLambda = _cascadeLambda;
    if (ImGui::SliderFloat("Cascade Lambda", &cascadeLambda, 0.01f, 1.0f))
    {
      _cascadeLambda = cascadeLambda;
    }

    bool shouldDrawCascadeLayers = _drawCascadeLayers;
    if (ImGui::Checkbox("Draw Cascade Layers", &shouldDrawCascadeLayers))
    {
      _drawCascadeLayers = shouldDrawCascadeLayers;
    }
  }
}

void ShadowMapRenderer::drawFrame(const std::shared_ptr<RenderDevice> &renderDevice,
                                  const std::vector<std::shared_ptr<Drawable>> &drawables,
                                  const std::vector<std::shared_ptr<Light>> &lights,
                                  const std::shared_ptr<Camera> &camera,
                                  const Vector3 &sceneMaxExtents, const Vector3 &sceneMinExtents)
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

  auto lightTransforms = calculateCascadeLightTransforms(camera, directionalLight);

  CascadeShadowMapData csmData;
  csmData.CascadeCount = lightTransforms.size();
  csmData.LightDirection = directionalLight->getDirection();
  csmData.LightTransforms[0] = lightTransforms[0];
  csmData.LightTransforms[1] = lightTransforms[1];
  csmData.LightTransforms[2] = lightTransforms[2];
  csmData.LightTransforms[3] = lightTransforms[3];
  csmData.DrawLayers = _drawCascadeLayers;
  csmData.SampleCount = _sampleCount;
  csmData.SampleSpread = _sampleSpread;

  auto cascadeLevels = calculateCascadeLevels(camera->getNear(), camera->getFar());
  csmData.CascadePlaneDistances[0].X = cascadeLevels[0];
  csmData.CascadePlaneDistances[1].X = cascadeLevels[1];
  csmData.CascadePlaneDistances[2].X = cascadeLevels[2];
  csmData.CascadePlaneDistances[3].X = cascadeLevels[3];
  _cascadeShadowBuffer->WriteData(0, sizeof(CascadeShadowMapData), &csmData, AccessType::WriteOnlyDiscard);

  _transformsBuffer->WriteData(0, sizeof(Matrix4) * 4, lightTransforms.data(), AccessType::WriteOnlyDiscard);

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

std::vector<float32> ShadowMapRenderer::calculateCascadeLevels(float32 nearClip, float32 farClip)
{
  float32 clipRange = farClip - nearClip;

  float32 minZ = nearClip + _minCascadeDistance * clipRange;
  float32 maxZ = nearClip + _maxCascadeDistance * clipRange;

  float32 range = maxZ - minZ;
  float32 ratio = maxZ / minZ;

  std::vector<float32> cascadeSplits;
  for (uint32 i = 0; i < _cascadeCount; ++i)
  {
    float32 p = (i + 1) / static_cast<float32>(_cascadeCount);
    float32 log = minZ * std::pow(ratio, p);
    float32 uniform = minZ + range * p;
    float32 d = _cascadeLambda * (log - uniform) + uniform;
    cascadeSplits.push_back(d);
  }

  return cascadeSplits;
}

std::vector<Matrix4> ShadowMapRenderer::calculateCascadeLightTransforms(const std::shared_ptr<Camera> &camera, const std::shared_ptr<Light> &directionalLight)
{
  std::vector<Matrix4> results;
  std::vector<Matrix4> projections = calculateCameraCascadeProjections(camera);
  for (uint32 i = 0; i < _cascadeCount; i++)
  {
    auto frustrumCorners = calculateFrustrumCorners(camera->getView(), projections[i]);
    Vector3 frustrumCenter = calculateFrustrumCenter(frustrumCorners);
    const float32 radius = calculateCascadeRadius(frustrumCorners, frustrumCenter);

    Vector3 maxExtents(radius, radius, radius);
    Vector3 minExtents = -maxExtents;
    Vector3 cascadeExtents = maxExtents - minExtents;

    Vector3 lightDirection = -directionalLight->getDirection();
    Vector3 shadowCameraPos = frustrumCenter + lightDirection * -minExtents.Z;
    Matrix4 shadowCameraView = Matrix4::LookAt(shadowCameraPos, frustrumCenter, Vector3::Up);

    Matrix4 shadowCameraProj = Matrix4::Orthographic(-radius, radius, -radius, radius, -cascadeExtents.Z, cascadeExtents.Z);
    Matrix4 shadowMatrix = shadowCameraProj * shadowCameraView;
    Vector4 shadowOrigin(0.0f, 0.0f, 0.0f, 1.0f);
    shadowOrigin = shadowMatrix * shadowOrigin;
    shadowOrigin = shadowOrigin * _shadowMapResolution / 2.0f;

    Vector4 roundedOrigin = Math::RoundToEven(shadowOrigin);
    Vector4 roundedOffset = roundedOrigin - shadowOrigin;
    roundedOffset = roundedOffset * (2.0f / _shadowMapResolution);
    roundedOffset.Z = 0.0f;
    roundedOffset.W = 0.0f;

    shadowCameraProj[3] += roundedOffset;

    results.push_back(shadowCameraProj * shadowCameraView);
  }
  return results;
}

std::vector<Matrix4> ShadowMapRenderer::calculateCameraCascadeProjections(const std::shared_ptr<Camera> &camera)
{
  Radian fov = camera->getFov();
  float32 aspect = camera->getAspectRatio();
  float32 nearPlane = camera->getNear();
  float32 farPlane = camera->getFar();

  std::vector<float32> cascadeLevels(calculateCascadeLevels(nearPlane, farPlane));

  std::vector<Matrix4> projections;
  projections.push_back(Matrix4::Perspective(fov, aspect, nearPlane, cascadeLevels[0]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[0], cascadeLevels[1]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[1], cascadeLevels[2]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[2], farPlane));
  return std::move(projections);
}

void ShadowMapRenderer::writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const
{
  ObjectBuffer objectBufferData;
  objectBufferData.Model = drawable->getMatrix();
  objectBufferData.ModelView = camera->getView() * objectBufferData.Model;
  objectBufferData.ModelViewProjection = camera->getProj() * objectBufferData.ModelView;
  _objectBuffer->WriteData(0, sizeof(ObjectBuffer), &objectBufferData, AccessType::WriteOnlyDiscard);
}
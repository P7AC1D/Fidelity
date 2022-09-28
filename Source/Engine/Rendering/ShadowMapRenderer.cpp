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

std::vector<float32> calculateCascadeLevels(float32 farPlane, const std::vector<float32> &cascadeRatios)
{
  return std::move(std::vector<float32>{farPlane * cascadeRatios[0], farPlane * cascadeRatios[1], farPlane * cascadeRatios[2], farPlane});
}

float32 calculateCascadeRadius(const std::vector<Vector4> &frustrumCorners)
{
  Assert::ThrowIfFalse(frustrumCorners.size() == 8, "Invalid size of supplied frustrum corners.");

  float32 length = (frustrumCorners[0] - frustrumCorners[6]).Length();
  return length / 2.0f;
}

float32 calculateTexelsPerUnitInWorldSpace(float32 shadowMapResolution, float32 cascadeRadius)
{
  return shadowMapResolution / (cascadeRadius * 2.0f);
}

std::vector<Matrix4> calculateCameraCascadeProjections(const std::shared_ptr<Camera> &camera, const std::vector<float32> &cascadeRatios)
{
  Radian fov = camera->getFar();
  float32 aspect = camera->getAspectRatio();
  float32 nearPlane = camera->getNear();
  float32 farPlane = camera->getFar();

  std::vector<float32> cascadeLevels(calculateCascadeLevels(farPlane, cascadeRatios));

  std::vector<Matrix4> projections;
  projections.push_back(Matrix4::Perspective(fov, aspect, nearPlane, cascadeLevels[0]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[0], cascadeLevels[1]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[1], cascadeLevels[2]));
  projections.push_back(Matrix4::Perspective(fov, aspect, cascadeLevels[2], farPlane));
  return std::move(projections);
}

Vector3 calculateFrustrumCenter(const std::vector<Vector4> &frustrumCorners)
{
  Vector3 center(Vector3::Zero);
  for (uint32 i = 0; i < 0; ++i)
  {
    center = center + Vector3(frustrumCorners[i][0], frustrumCorners[i][1], frustrumCorners[i][2]);
  }
  return center * (1.0f / 8.0f);
}

std::vector<Vector4> calculateFrustrumCorners(const Matrix4 &view, const Matrix4 &projection)
{
  std::vector<Vector4> frustrumCorners = {
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

  for (uint32 i = 0; i < 8; i++)
  {
    frustrumCorners[i] = projViewInvs * frustrumCorners[i];
    frustrumCorners[i] = frustrumCorners[i] / frustrumCorners[i].W;
  }

  return std::move(frustrumCorners);
}

Matrix4 calculateLightView(const std::vector<Vector4> &frustrumCorners,
                           const std::shared_ptr<Light> &directionalLight,
                           float32 shadowMapResolution)
{
  // To reduce shadow shimmering when moving the camera
  // 1. Scale light view by a radius to ensure constant size
  const float32 radius = calculateCascadeRadius(frustrumCorners);
  const float32 texelsPerUnit = calculateTexelsPerUnitInWorldSpace(shadowMapResolution, radius);
  const Vector3 lightDirection = directionalLight->getDirection();

  Matrix4 lookAt(Matrix4::LookAt(Vector3::Zero, -lightDirection, Vector3(0.0f, 1.0f, 0.0f)));
  lookAt = lookAt * Matrix4::Scaling(Vector3(texelsPerUnit));

  Matrix4 lookAtInv(lookAt.Inverse());

  // 2. Move center in texel-size increments; Transform back into original space using inverse.
  Vector3 fustrumCenter = calculateFrustrumCenter(frustrumCorners);
  fustrumCenter = lookAt * fustrumCenter;
  fustrumCenter.X = std::floorf(fustrumCenter.X);
  fustrumCenter.Y = std::floorf(fustrumCenter.Y);
  fustrumCenter = lookAtInv * fustrumCenter;

  Vector3 eye = fustrumCenter - (lightDirection * radius * 2.0f);
  return Matrix4::LookAt(eye, fustrumCenter, Vector3::Up);
}

Matrix4 calculateLightProjection(float32 radius, float32 zMulti)
{
  return Matrix4::Orthographic(-radius, radius, -radius, radius, -radius * zMulti, radius * zMulti);
}

std::vector<Matrix4> calculateCascadeLightTransforms(const std::shared_ptr<Camera> &camera,
                                                     const std::shared_ptr<Light> &directionalLight,
                                                     uint32 shadowMapResolution,
                                                     const std::vector<float32> &cascadeRatios,
                                                     float32 zMulti)
{
  std::vector<Matrix4> results;
  std::vector<Matrix4> projections = calculateCameraCascadeProjections(camera, cascadeRatios);
  for (uint32 i = 0; i < 4; i++)
  {
    auto frustrumCorners = calculateFrustrumCorners(camera->getView(), projections[i]);
    Matrix4 lightView = calculateLightView(frustrumCorners, directionalLight, shadowMapResolution);
    Matrix4 lightProjection = calculateLightProjection(calculateCascadeRadius(frustrumCorners), zMulti);
    results.push_back(lightProjection * lightView);
  }
  return std::move(results);
}

ShadowMapRenderer::ShadowMapRenderer() : _zMulti(3.0),
                                         _shadowMapResolution(4096),
                                         _cascadeCount(4),
                                         _drawCascadeLayers(false)
{
  _cascadeRatios.push_back(0.1f);
  _cascadeRatios.push_back(0.2f);
  _cascadeRatios.push_back(0.3f);
  _cascadeRatios.push_back(0.4f);
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

    bool shouldDrawCascadeLayers = _drawCascadeLayers;
    if (ImGui::Checkbox("Draw Cascade Layers", &shouldDrawCascadeLayers))
    {
      _drawCascadeLayers = shouldDrawCascadeLayers;
    }

    if (ImGui::TreeNodeEx("Cascade Ratios", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))
    {
      for (uint32 i = 0; i < _cascadeRatios.size(); i++)
      {
        float32 value = _cascadeRatios[i];
        if (ImGui::DragFloat(std::string("Layer " + std::to_string(i)).c_str(), &value, 0.001f, 0.001f, 1.0f))
        {
          _cascadeRatios[i] = value;
        }
      }
      ImGui::TreePop();
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

  auto lightTransforms = calculateCascadeLightTransforms(camera, directionalLight, _shadowMapResolution, _cascadeRatios, _zMulti);

  CascadeShadowMapData csmData;
  csmData.CascadeCount = lightTransforms.size();
  csmData.LightDirection = directionalLight->getDirection();
  csmData.LightTransforms[0] = lightTransforms[0];
  csmData.LightTransforms[1] = lightTransforms[1];
  csmData.LightTransforms[2] = lightTransforms[2];
  csmData.LightTransforms[3] = lightTransforms[3];
  csmData.DrawLayers = _drawCascadeLayers;

  auto cascadeLevels = calculateCascadeLevels(camera->getFar(), _cascadeRatios);
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

void ShadowMapRenderer::writeObjectConstantData(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Camera> &camera) const
{
  ObjectBuffer objectBufferData;
  objectBufferData.Model = drawable->getMatrix();
  objectBufferData.ModelView = camera->getView() * objectBufferData.Model;
  objectBufferData.ModelViewProjection = camera->getProj() * objectBufferData.ModelView;
  _objectBuffer->WriteData(0, sizeof(ObjectBuffer), &objectBufferData, AccessType::WriteOnlyDiscard);
}
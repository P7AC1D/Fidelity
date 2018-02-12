#include "Renderer.h"

#include "../Maths/Colour.hpp"
#include "../Maths/Matrix4.hpp"
#include "../UI/Panel.h"
#include "../UI/UIManager.h"
#include "../SceneManagement/Light.h"
#include "../SceneManagement/WorldObject.h"
#include "../Shaders/DirDepthPassShader.hpp"
#include "../Shaders/GeometryPassShader.hpp"
#include "../Shaders/DirLightingPassShader.hpp"
#include "ConstantBuffer.h"
#include "CubeMap.h"
#include "Material.h"
#include "OpenGL.h"
#include "Renderable.hpp"
#include "RenderTarget.hpp"
#include "Shader.h"
#include "ShaderCollection.h"
#include "Texture.h"
#include "StaticMesh.h"
#include "VertexBuffer.h"

using namespace Rendering;
using namespace UI;

namespace Rendering
{
enum class RenderingTechnique
{
  Coloured = 0,
  Textured = 1,
};

enum class VertexArribLocation
{
  Position = 0,
  Normal = 1,
  TexCoord = 2,
  Tangent = 3,
  Bitangent = 4,
};

enum class UniformBindingPoint
{
  Transforms = 0,
  Light = 1,
};

Renderer::Renderer(int32 renderWidth, int32 renderHeight) :
  _shaderCollection(new ShaderCollection),
  _renderWidth(renderWidth),
  _renderHeight(renderHeight),
  _ambientLight(Vector3::Identity)
{
}

Renderer::~Renderer()
{
}

void Renderer::SetClearColour(const Colour& colour)
{
  glClearColor(colour[0], colour[1], colour[2], 0.0f);
}

void Renderer::DrawScene(OrbitalCamera& camera)
{
  UploadCameraData(camera);

  auto lightSpaceTransform = BuildLightSpaceTransform(_directionalLights[0]);
  ExecuteDirectionalLightDepthPass(lightSpaceTransform, _shadowResolution);

  auto viewDirection = camera.GetDirection();
  ExecuteGeometryPass(viewDirection);
  ExecuteLightingPass(lightSpaceTransform, viewDirection);

  _renderables.clear();
  _pointLights.clear();
  _directionalLights.clear();
}

void Renderer::Draw(uint32 vertexCount, uint32 vertexOffset)
{
  GLCall(glDrawArrays(GL_TRIANGLES, vertexOffset, vertexCount));
}

void Renderer::DrawIndexed(uint32 indexCount)
{
  GLCall(glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0));
}

bool Renderer::Initialize()
{
#ifdef _WIN32
  glewExperimental = GL_TRUE;
  GLenum error = glewInit();
  if (error != GLEW_OK)
  {
    //LOG_ERROR << "Could not initialize GLEW: " + std::string((char*)glewGetErrorString(error));
    return false;
  }
#endif

  GLint result = -1;
  GLCall(glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &result));

  GLCall(glEnable(GL_DEPTH_TEST));
  GLCall(glEnable(GL_CULL_FACE));
  GLCall(glDisable(GL_BLEND));
  GLCall(glCullFace(GL_BACK));
  GLCall(glFrontFace(GL_CCW));

  GLCall(glViewport(0, 0, _renderWidth, _renderHeight));
  GLCall(glClearColor(0.25f, 0.25f, 0.25f, 0.0f));

  _cameraBuffer.reset(new ConstantBuffer(144));
  _lightBuffer.reset(new ConstantBuffer(52));

  RenderTargetDesc gBufferDesc;
  gBufferDesc.Width = _renderWidth;
  gBufferDesc.Height = _renderHeight;
  gBufferDesc.ColourBufferCount = 3;
  gBufferDesc.EnableDepthBuffer = true;
  _gBuffer.reset(new RenderTarget(gBufferDesc));

  RenderTargetDesc dirDepthBufferDesc;
  dirDepthBufferDesc.Width = _shadowResolution;
  dirDepthBufferDesc.Height = _shadowResolution;
  dirDepthBufferDesc.ColourBufferCount = 0;
  dirDepthBufferDesc.EnableDepthBuffer = true;
  _depthBuffer.reset(new RenderTarget(dirDepthBufferDesc));

  if (!_quadVertexData)
  {
    std::vector<Vector2> quadVertexData
    {
      Vector2(-1.0f, -1.0f),
      Vector2(0.0f, 0.0f),

      Vector2(1.0f, -1.0f),
      Vector2(1.0f, 0.0f),

      Vector2(-1.0f, 1.0f),
      Vector2(0.0f, 1.0f),

      Vector2(1.0f, -1.0f),
      Vector2(1.0f, 0.0f),

      Vector2(1.0f, 1.0f),
      Vector2(1.0f, 1.0f),

      Vector2(-1.0f, 1.0f),
      Vector2(0.0f, 1.0f)
    };

    _quadVertexData.reset(new VertexBuffer());
    _quadVertexData->UploadData<Vector2>(quadVertexData, BufferUsage::Static);

    auto posLoc = static_cast<uint32>(VertexArribLocation::Position);
    auto uvLoc = static_cast<uint32>(VertexArribLocation::TexCoord);

    GLCall(glBindVertexArray(_quadVertexData->GetId()));
    GLCall(glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 16, nullptr));
    GLCall(glEnableVertexAttribArray(posLoc));
    GLCall(glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 16, (void*)8));
    GLCall(glEnableVertexAttribArray(uvLoc));
    GLCall(glBindVertexArray(0));
  }

  return true;
}

void Renderer::SetVertexAttribPointers(StaticMesh* staticMesh, int32 stride)
{
  if (!staticMesh)
  {
    return;
  }

  GLCall(glBindVertexArray(staticMesh->_vertexBuffer->GetId()));
  if (staticMesh->_vertexDataFormat & VertexDataFormat::Position)
  {
    auto positionLocation = static_cast<int32>(VertexArribLocation::Position);
    GLCall(glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, stride, nullptr));
    GLCall(glEnableVertexAttribArray(positionLocation));
  }
  if (staticMesh->_vertexDataFormat & VertexDataFormat::Normal)
  {
    auto normalLocation = static_cast<int32>(VertexArribLocation::Normal);
    GLCall(glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, stride, (void*)12));
    GLCall(glEnableVertexAttribArray(normalLocation));
  }
  if (staticMesh->_vertexDataFormat & VertexDataFormat::Uv)
  {
    auto uvLocation = static_cast<int32>(VertexArribLocation::TexCoord);
    GLCall(glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, stride, (void*)24));
    GLCall(glEnableVertexAttribArray(uvLocation));
  }
  if (staticMesh->_vertexDataFormat & VertexDataFormat::Tangent)
  {
    auto uvLocation = static_cast<int32>(VertexArribLocation::Tangent);
    GLCall(glVertexAttribPointer(uvLocation, 3, GL_FLOAT, GL_FALSE, stride, (void*)32));
    GLCall(glEnableVertexAttribArray(uvLocation));
  }
  if (staticMesh->_vertexDataFormat & VertexDataFormat::Bitanget)
  {
    auto uvLocation = static_cast<int32>(VertexArribLocation::Bitangent);
    GLCall(glVertexAttribPointer(uvLocation, 3, GL_FLOAT, GL_FALSE, stride, (void*)44));
    GLCall(glEnableVertexAttribArray(uvLocation));
  }
  GLCall(glBindVertexArray(0));
}
  
void Renderer::SetViewport(int32 width, int32 height)
{
  glViewport(0, 0, width, height);
}

void Renderer::UploadCameraData(OrbitalCamera& camera)
{
  _cameraBuffer->UploadData(0, 64, &camera.GetProjMat()[0]);
  _cameraBuffer->UploadData(64, 64, &camera.GetViewMat()[0]);
  _cameraBuffer->UploadData(128, 12, &camera.GetPos()[0]);
}

void Renderer::UploadPointLightData(const Light& pointLight)
{
  _lightBuffer->UploadData(0, 12, &pointLight.GetPosition()[0]);
  _lightBuffer->UploadData(32, 12, &(pointLight.GetColour().ToVec3())[0]);

  float32 radius = pointLight.GetRadius();
  _lightBuffer->UploadData(44, 4, &radius);
}

void Renderer::UploadDirectionalLightData(const Light& directionalLight)
{
  _lightBuffer->UploadData(16, 12, &directionalLight.GetDirection()[0]);
  _lightBuffer->UploadData(32, 12, &(directionalLight.GetColour().ToVec3())[0]);
}

void Renderer::ExecuteDirectionalLightDepthPass(const Matrix4& lightSpaceTransform, uint32 shadowResolution)
{
  SetDepthTest(true);
  GLCall(glDepthFunc(GL_LESS));

  SetViewport(shadowResolution, shadowResolution);
  _depthBuffer->Enable();
  ClearBuffer(ClearType::Depth);

  auto shader = _shaderCollection->GetShader<DirDepthPassShader>();
  shader->SetLightSpaceTransform(lightSpaceTransform);
  for (auto& renderable : _renderables)
  {
    auto meshCount = renderable.Renderable->GetMeshCount();
    for (size_t i = 0; i < meshCount; i++)
    {
      auto& staticMesh = renderable.Renderable->GetMeshAtIndex(i);
      shader->SetModelSpaceTransform(renderable.Transform->Get());

      shader->Apply();
      staticMesh.Draw();
    }
  }
  _gBuffer->Disable();
  SetViewport(_renderWidth, _renderHeight);
}

void Renderer::ExecuteGeometryPass(const Vector3& viewDirection)
{
  _gBuffer->Enable();
  ClearBuffer(ClearType::All);

  //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

  auto shader = _shaderCollection->GetShader<GeometryPassShader>();
  shader->SetTransformsUniformbuffer(_cameraBuffer);
  shader->SetViewDirection(viewDirection);
  for (auto& renderable : _renderables)
  {    
    auto meshCount = renderable.Renderable->GetMeshCount();
    for (size_t i = 0; i < meshCount; i++)
    {
      auto& staticMesh = renderable.Renderable->GetMeshAtIndex(i);
      shader->SetModelTransform(renderable.Transform->Get());
      shader->SetMaterialProperties(staticMesh.GetMaterial());

      shader->Apply();
      staticMesh.Draw();
    }
  }
  _gBuffer->Disable();

  //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

void Renderer::ExecuteLightingPass(const Matrix4& lightSpaceTransform, const Vector3& viewDirection)
{
  SetDepthTest(false);
  ClearBuffer(ClearType::All);

  auto shader = _shaderCollection->GetShader<DirLightingPassShader>();
  shader->SetViewDirection(viewDirection);
  shader->SetDirectionalLight(_directionalLights[0]);
  shader->SetGeometryBuffer(_gBuffer);
  shader->SetDirLightDepthBuffer(_depthBuffer);
  shader->SetLightSpaceTransform(lightSpaceTransform);
  shader->Apply();

  _quadVertexData->Bind();
  Draw(6);
}

void Renderer::ClearBuffer(ClearType clearType)
{
  switch (clearType)
  {
    case ClearType::Color:
      GLCall(glClear(GL_COLOR_BUFFER_BIT));
      break;
    case ClearType::Depth:
      GLCall(glClear(GL_DEPTH_BUFFER_BIT));
      break;
    case ClearType::All:
      GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
      break;
  }
}

void Renderer::SetDepthTest(bool enable)
{
  if (enable)
  {
    GLCall(glEnable(GL_DEPTH_TEST));
  }
  else
  {
    GLCall(glDisable(GL_DEPTH_TEST));
  }
}

Matrix4 Renderer::BuildLightSpaceTransform(const Light& directionalLight)
{
  auto lightProj = Matrix4::Orthographic(-100.0f, 100.0f, -100.0f, 100.0f, -100.0f, 100.0f);
  auto lightView = Matrix4::LookAt(-directionalLight.GetDirection(), Vector3::Zero, Vector3(0.0f, 1.0f, 0.0f));
  return lightProj * lightView; 
}
}

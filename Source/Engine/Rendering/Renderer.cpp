#include "Renderer.h"

#include "../Maths/Colour.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Overlay/TextOverlay.hpp"
#include "../SceneManagement/Light.h"
#include "../SceneManagement/SkyBox.hpp"
#include "../SceneManagement/WorldObject.h"
#include "../Shaders/DirDepthPassShader.hpp"
#include "../Shaders/GeometryPassShader.hpp"
#include "../Shaders/DirLightingPassShader.hpp"
#include "../Shaders/SkyBoxShader.hpp"
#include "../Shaders/TextOverlayShader.hpp"
#include "ConstantBuffer.h"
#include "Material.h"
#include "OpenGL.h"
#include "Renderable.hpp"
#include "RenderTarget.hpp"
#include "Shader.h"
#include "ShaderCollection.h"
#include "Texture.hpp"
#include "StaticMesh.h"
#include "VertexBuffer.h"

namespace Rendering
{
GLenum ToBlendType(BlendType blendType)
{
  switch (blendType)
  {
    case BlendType::SrcAlpha: return GL_SRC_ALPHA;
    case BlendType::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
    default: GL_ONE;
  }
}

enum class UniformBindingPoint
{
  Transforms = 0,
  Light = 1,
};

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
  DrawSkyBox();

  _renderables.clear();
  _pointLights.clear();
  _directionalLights.clear();
}

void Renderer::DrawOverlay()
{
  DisableStencilTest();
  DisableDepthTest();
  
  EnableBlend(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);

  auto shader = _shaderCollection->GetShader<TextOverlayShader>();  
  for (auto& textOverlay : _textOverlays)
  {
    shader->SetFontAtlas(textOverlay->GetDesc().Atlas);
    shader->SetTextColour(textOverlay->GetDesc().TextColour);
    shader->Apply();
    textOverlay->Draw();
  }
  
  DisableBlend();
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
    
  GLCall(glCullFace(GL_BACK));
  GLCall(glFrontFace(GL_CCW));

  SetClearColour(Colour::Black);

  _cameraBuffer.reset(new ConstantBuffer(144));
  _lightBuffer.reset(new ConstantBuffer(52));

  RenderTargetDesc gBufferDesc;
  gBufferDesc.Width = _renderWidth;
  gBufferDesc.Height = _renderHeight;
  gBufferDesc.ColourBufferCount = 3;
  gBufferDesc.EnableStencilBuffer = true;
  _gBuffer.reset(new RenderTarget(gBufferDesc));

  RenderTargetDesc dirDepthBufferDesc;
  dirDepthBufferDesc.Width = _shadowResolution;
  dirDepthBufferDesc.Height = _shadowResolution;
  dirDepthBufferDesc.ColourBufferCount = 0;
  dirDepthBufferDesc.EnableStencilBuffer = false;
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
    _quadVertexData->PushVertexAttrib(VertexAttribType::Vec2);
    _quadVertexData->PushVertexAttrib(VertexAttribType::Vec2);
    _quadVertexData->UploadData<Vector2>(quadVertexData, BufferUsage::Static);
  }

  return true;
}
  
void Renderer::SetRenderDimensions(uint32 width, uint32 height)
{
  _renderWidth = width;
  _renderHeight = height;
}

void Renderer::EnableBlend(BlendType source, BlendType destination)
{
  GLCall(glEnable(GL_BLEND));
  GLCall(glBlendFunc(ToBlendType(source), ToBlendType(destination)));
}

void Renderer::DisableBlend()
{
  GLCall(glDisable(GL_BLEND));
}
  
void Renderer::SetViewport(uint32 width, uint32 height)
{
  GLCall(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
}

Renderer::Renderer() :
  _shaderCollection(new ShaderCollection),
  _renderWidth(0),
  _renderHeight(0),
  _ambientLight(Vector3::Identity)
{
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
  EnableDepthTest();
  DisableStencilTest();

  SetViewport(shadowResolution, shadowResolution);

  _depthBuffer->BindForDraw();
  ClearBuffer(ClearType::CT_Depth | ClearType::CT_Stencil);

  auto shader = _shaderCollection->GetShader<DirDepthPassShader>();
  shader->SetLightSpaceTransform(lightSpaceTransform);
  for (auto& renderable : _renderables)
  {
    auto meshCount = renderable.Renderable->GetMeshCount();
    for (size_t i = 0; i < meshCount; i++)
    {
      auto staticMesh = renderable.Renderable->GetMeshAtIndex(i);
      shader->SetModelSpaceTransform(renderable.Transform->Get());

      shader->Apply();
      staticMesh->Draw();
    }
  }
  _gBuffer->Unbind();
  SetViewport(_renderWidth, _renderHeight);
}

void Renderer::ExecuteGeometryPass(const Vector3& viewDirection)
{
  EnableStencilTest();

  _gBuffer->Bind();
  GLCall(glStencilFunc(GL_ALWAYS, 1, 0xFF));
  GLCall(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
  GLCall(glStencilMask(0xFF));
  ClearBuffer(CT_Colour | CT_Depth | CT_Stencil);

  //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

  auto shader = _shaderCollection->GetShader<GeometryPassShader>();
  shader->SetTransformsUniformbuffer(_cameraBuffer);
  shader->SetViewDirection(viewDirection);
  for (auto renderable : _renderables)
  {    
    auto meshCount = renderable.Renderable->GetMeshCount();
    for (size_t i = 0; i < meshCount; i++)
    {
      auto staticMesh = renderable.Renderable->GetMeshAtIndex(i);
      shader->SetModelTransform(renderable.Transform->Get());
      shader->SetMaterialProperties(staticMesh->GetMaterial());

      shader->Apply();
      staticMesh->Draw();
    }
  }
  _gBuffer->Unbind();

  //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

void Renderer::ExecuteLightingPass(const Matrix4& lightSpaceTransform, const Vector3& viewDirection)
{
  DisableDepthTest();
  DisableStencilTest();
  ClearBuffer(ClearType::CT_Colour);

  auto shader = _shaderCollection->GetShader<DirLightingPassShader>();
  shader->SetViewDirection(viewDirection);
  shader->SetDirectionalLight(_directionalLights[0]);
  shader->SetGeometryBuffer(_gBuffer);
  shader->SetDirLightDepthBuffer(_depthBuffer);
  shader->SetLightSpaceTransform(lightSpaceTransform);
  shader->Apply();

  _quadVertexData->Apply();
  Draw(6);
}

void Renderer::DrawSkyBox()
{
  if (!_skyBox)
  {
    return;
  }
  DisableDepthTest();
  EnableStencilTest();

  _gBuffer->BindForRead();
  GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
  GLCall(glBlitFramebuffer(0, 0, _renderWidth, _renderHeight, 0, 0, _renderWidth, _renderHeight, GL_STENCIL_BUFFER_BIT, GL_NEAREST));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  GLCall(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
  
  GLCall(glDisable(GL_CULL_FACE));

  auto shader = _shaderCollection->GetShader<SkyBoxShader>();
  shader->SetTransformBuffer(_cameraBuffer);
  shader->SetSkyBox(_skyBox);
  shader->Apply();

  _skyBox->Draw();

  GLCall(glEnable(GL_CULL_FACE));
}

void Renderer::ClearBuffer(uint32 clearType)
{
  GLbitfield clearTarget = 0;
  if (clearType & ClearType::CT_Colour)
  {
    clearTarget |= GL_COLOR_BUFFER_BIT;
  }

  if (clearType & ClearType::CT_Depth)
  {
    clearTarget |= GL_DEPTH_BUFFER_BIT;
  }

  if (clearType & ClearType::CT_Stencil)
  {
    clearTarget |= GL_STENCIL_BUFFER_BIT;
  }

  GLCall(glClear(clearTarget));
}

void Renderer::EnableDepthTest()
{
  GLCall(glEnable(GL_DEPTH_TEST));
}

void Renderer::DisableDepthTest()
{
  GLCall(glDisable(GL_DEPTH_TEST));
}

void Renderer::EnableStencilTest()
{
  GLCall(glEnable(GL_STENCIL_TEST));
}

void Renderer::DisableStencilTest()
{
  GLCall(glDisable(GL_STENCIL_TEST));
}

Matrix4 Renderer::BuildLightSpaceTransform(const Light& directionalLight)
{
  auto lightProj = Matrix4::Orthographic(-100.0f, 100.0f, -100.0f, 100.0f, -100.0f, 100.0f);
  auto lightView = Matrix4::LookAt(-directionalLight.GetDirection(), Vector3::Zero, Vector3(0.0f, 1.0f, 0.0f));
  return lightProj * lightView; 
}
}
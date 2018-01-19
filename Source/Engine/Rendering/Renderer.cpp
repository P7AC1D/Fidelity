#include "Renderer.h"

#include "../Components/PointLight.h"
#include "../UI/Panel.h"
#include "../UI/UIManager.h"
#include "../SceneManagement/WorldObject.h"
#include "ConstantBuffer.h"
#include "CubeMap.h"
#include "Material.h"
#include "Renderable.hpp"
#include "Shader.h"
#include "ShaderCollection.h"
#include "Texture.h"
#include "StaticMesh.h"
#include "VertexBuffer.h"

using namespace Components;
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
  _shaderCollection(new ShaderCollection("./../../Source/Engine/Shaders/")),
  _renderWidth(renderWidth),
  _renderHeight(renderHeight)
{
}

Renderer::~Renderer()
{
}

void Renderer::SetViewport(int32 renderWidth, int32 renderHeight)
{
  _renderWidth = renderWidth;
  _renderHeight = renderHeight;
  glViewport(0, 0, _renderWidth, _renderHeight);
}

void Renderer::SetClearColour(const Vector3& colour)
{
  glClearColor(colour[0], colour[1], colour[2], 0.0f);
}

void Renderer::PushRenderable(std::shared_ptr<Renderable> renderable, std::shared_ptr<Transform> transform)
{
  _renderables.emplace_back(renderable, transform);
}

void Renderer::PushPointLight(std::shared_ptr<PointLight> pointLight)
{
  _pointLights.push_back(pointLight);
}

void Renderer::DrawScene(OrbitalCamera& camera)
{
  auto shader = _shaderCollection->GetShader("Standard.shader");
  shader->Bind();
  shader->SetVec3("u_ambientColour", Vector3::Identity);
  shader->BindUniformBlock(shader->GetUniformBlockIndex("Transforms"), static_cast<int32>(UniformBindingPoint::Transforms), _cameraBuffer->_uboId, _cameraBuffer->_sizeBytes);
  shader->BindUniformBlock(shader->GetUniformBlockIndex("PointLight"), static_cast<int32>(UniformBindingPoint::Light), _lightBuffer->_uboId, _lightBuffer->_sizeBytes);

  ClearBuffer(ClearType::All);
  UploadCameraData(camera);

  GLCall(glDepthFunc(GL_LESS));
  for (auto light : _pointLights)
  {
    UploadLightData(light);

    for (auto& renderable : _renderables)
    {
      shader->SetBool("u_diffuseMappingEnabled", false);

      auto meshCount = renderable.Renderable->GetMeshCount();
      for (size_t i = 0; i < meshCount; i++)
      {
        auto& staticMesh = renderable.Renderable->GetMeshAtIndex(i);
        shader->SetMat4("u_model", renderable.Transform->Get());

        auto material = staticMesh.GetMaterial();
        if (material->HasTexture("DiffuseMap"))
        {
          auto diffuseMap = material->GetTexture("DiffuseMap");
          glActiveTexture(GL_TEXTURE0);
          diffuseMap->Bind();
          shader->SetInt("u_material.DiffuseMap", 0);
          shader->SetBool("u_diffuseMappingEnabled", true);
        }

        shader->SetFloat("u_material.SpecularExponent", material->GetSpecularExponent());
        shader->SetVec3("u_material.AmbientColour", material->GetAmbientColour());
        shader->SetVec3("u_material.DiffuseColour", material->GetDiffuseColour());
        shader->SetVec3("u_material.SpecularColour", material->GetSpecularColour());

        auto vertexData = staticMesh.GetVertexData();
        GLCall(glBindVertexArray(vertexData->_vaoId));
        GLCall(glDrawArrays(GL_TRIANGLES, 0, staticMesh._vertexCount));
        GLCall(glBindVertexArray(0));
      }
    }

    GLCall(glDepthFunc(GL_EQUAL));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendEquation(GL_FUNC_ADD));
    GLCall(glBlendFunc(GL_ONE, GL_ONE));
  }
  GLCall(glDisable(GL_BLEND));

  _renderables.clear();
  _pointLights.clear();
}

void Renderer::DrawUI(std::vector<std::shared_ptr<Panel>> panelCollection)
{
  if (!_guiQuadVertexData)
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

    _guiQuadVertexData.reset(new VertexBuffer());
    _guiQuadVertexData->UploadData<Vector2>(quadVertexData, BufferUsage::Static);

    auto posLoc = static_cast<uint32>(VertexArribLocation::Position);
    auto uvLoc = static_cast<uint32>(VertexArribLocation::TexCoord);

    GLCall(glBindVertexArray(_guiQuadVertexData->_vaoId));
    GLCall(glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 16, nullptr));
    GLCall(glEnableVertexAttribArray(posLoc));
    GLCall(glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 16, (void*)8));
    GLCall(glEnableVertexAttribArray(uvLoc));
    GLCall(glBindVertexArray(0));
  }
  
  auto shader = _shaderCollection->GetShader("Gui.glsl");
  if (!shader)
  {
    return;
  }
  shader->Bind();

  for (auto panel : panelCollection)
  {
    if (panel->_texture)
    {
      GLCall(glActiveTexture(GL_TEXTURE0));
      panel->_texture->Bind();
      shader->SetInt("textureMap", GL_TEXTURE0);
    }

    float32 xScale = panel->_width / static_cast<float32>(_renderWidth);
    float32 yScale = panel->_height / static_cast<float32>(_renderHeight);
    float32 xOffset = 2.0f * panel->_xPos / _renderWidth - (1.0f - xScale);
    float32 yOffset = 2.0f * panel->_yPos / _renderHeight - (1.0f - yScale);

    shader->SetFloat("xScale", xScale);
    shader->SetFloat("yScale", yScale);
    shader->SetFloat("xOffset", xOffset);
    shader->SetFloat("yOffset", yOffset);
    
    GLCall(glBindVertexArray(_guiQuadVertexData->_vaoId));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
    GLCall(glBindVertexArray(0));
  }
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

  GLCall(glEnable(GL_DEPTH_TEST));
  GLCall(glEnable(GL_CULL_FACE));
  GLCall(glCullFace(GL_BACK));
  GLCall(glFrontFace(GL_CCW));

  GLCall(glViewport(0, 0, _renderWidth, _renderHeight));
  GLCall(glClearColor(0.25f, 0.25f, 0.25f, 0.0f));

  _cameraBuffer.reset(new ConstantBuffer(144));
  _lightBuffer.reset(new ConstantBuffer(32));

  return true;
}

void Renderer::SetVertexAttribPointers(StaticMesh* staticMesh, int32 stride)
{
  if (!staticMesh)
  {
    return;
  }

  GLCall(glBindVertexArray(staticMesh->_vertexBuffer->_vaoId));
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
  GLCall(glBindVertexArray(0));
}

void Renderer::UploadCameraData(OrbitalCamera& camera)
{
  _cameraBuffer->UploadData(0, 64, &camera.GetProjMat()[0]);
  _cameraBuffer->UploadData(64, 64, &camera.GetViewMat()[0]);
  _cameraBuffer->UploadData(128, 12, &camera.GetPos()[0]);
}

void Renderer::UploadLightData(std::shared_ptr<PointLight> pointLight)
{
  _lightBuffer->UploadData(0, 12, &pointLight->GetPosition()[0]);
  _lightBuffer->UploadData(16, 12, &pointLight->GetColour()[0]);

  float32 radius = pointLight->GetRadius();
  _lightBuffer->UploadData(28, 4, &radius);
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
}

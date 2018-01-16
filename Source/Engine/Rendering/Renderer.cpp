#include "Renderer.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif
#ifdef _WIN32
#include <GL/glew.h>
#endif

#include "../Components/Model.hpp"
#include "../Components/Transform.h"
#include "../Components/PointLight.h"
#include "../UI/Panel.h"
#include "../UI/UIManager.h"
#include "../SceneManagement/OrbitalCamera.h"
#include "../SceneManagement/Scene.h"
#include "../SceneManagement/SceneNode.h"
#include "../SceneManagement/WorldObject.h"
#include "ConstantBuffer.h"
#include "CubeMap.h"
#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "ShaderCollection.h"
#include "StaticMesh.h"
#include "VertexBuffer.h"

using namespace Components;
using namespace Rendering;
using namespace UI;
using namespace SceneManagement;

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
  Tangent = 2,
  Bitangent = 3,
  Uv = 4,
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
{}

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

void Renderer::PreRender()
{
  ClearBuffer(ClearType::All);
}

void Renderer::DrawScene(std::shared_ptr<Scene> scene)
{
  glDepthFunc(GL_LESS);

  UploadCameraData(scene->GetCamera());
  DrawSkyBox(scene);

  auto rootNode = scene->GetRootNode();
  auto models = rootNode->GetAllObjectsWithComponent("Model");
  auto lightObjects = rootNode->GetAllObjectsWithComponent("PointLight");
  for (auto lightObject : lightObjects)
  {    
    UploadLightData(lightObject);
    for (auto model : models)
    {
      DrawTexturedObjects(models, scene->GetAmbientLight());
    }
    glDepthFunc(GL_EQUAL);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
  }
  glDisable(GL_BLEND);
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
    auto uvLoc = static_cast<uint32>(VertexArribLocation::Uv);

    glBindVertexArray(_guiQuadVertexData->_vaoId);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 16, nullptr);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);
    glEnableVertexAttribArray(uvLoc);
    glBindVertexArray(0);
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
      glActiveTexture(GL_TEXTURE0);
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
    
    glBindVertexArray(_guiQuadVertexData->_vaoId);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glViewport(0, 0, _renderWidth, _renderHeight);
  glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
  return true;
}

void Renderer::SetVertexAttribPointers(StaticMesh* staticMesh, int32 stride)
{
  if (!staticMesh)
  {
    return;
  }

  glBindVertexArray(staticMesh->_vertexBuffer->_vaoId);
  if (staticMesh->_vertexDataFormat & VertexDataFormat::Position)
  {
    auto positionLocation = static_cast<int32>(VertexArribLocation::Position);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(positionLocation);
  }
  if (staticMesh->_vertexDataFormat & VertexDataFormat::Normal)
  {
    auto normalLocation = static_cast<int32>(VertexArribLocation::Normal);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, stride, (void*)12);
    glEnableVertexAttribArray(normalLocation);
  }
  if (staticMesh->_vertexDataFormat & VertexDataFormat::Uv)
  {
    auto uvLocation = static_cast<int32>(VertexArribLocation::Uv);
    glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, stride, (void*)24);
    glEnableVertexAttribArray(uvLocation);
  }
  glBindVertexArray(0);
}

void Renderer::DrawSkyBox(std::shared_ptr<SceneManagement::Scene> scene)
{
  if (!scene->GetSkyBox())
  {
    return;
  }

  if (!_skyBoxVertexData)
  {
    static std::vector<Vector3> skyBoxVertices =
    {
      Vector3(-1.0f,  1.0f, -1.0f),
      Vector3(-1.0f, -1.0f, -1.0f),
      Vector3(1.0f, -1.0f, -1.0f),
      Vector3(1.0f, -1.0f, -1.0f),
      Vector3(1.0f,  1.0f, -1.0f),
      Vector3(-1.0f,  1.0f, -1.0f),

      Vector3(-1.0f, -1.0f,  1.0f),
      Vector3(-1.0f, -1.0f, -1.0f),
      Vector3(-1.0f,  1.0f, -1.0f),
      Vector3(-1.0f,  1.0f, -1.0f),
      Vector3(-1.0f,  1.0f,  1.0f),
      Vector3(-1.0f, -1.0f,  1.0f),

      Vector3(1.0f, -1.0f, -1.0f),
      Vector3(1.0f, -1.0f,  1.0f),
      Vector3(1.0f,  1.0f,  1.0f),
      Vector3(1.0f,  1.0f,  1.0f),
      Vector3(1.0f,  1.0f, -1.0f),
      Vector3(1.0f, -1.0f, -1.0f),

      Vector3(-1.0f, -1.0f,  1.0f),
      Vector3(-1.0f,  1.0f,  1.0f),
      Vector3(1.0f,  1.0f,  1.0f),
      Vector3(1.0f,  1.0f,  1.0f),
      Vector3(1.0f, -1.0f,  1.0f),
      Vector3(-1.0f, -1.0f,  1.0f),

      Vector3(-1.0f,  1.0f, -1.0f),
      Vector3(1.0f,  1.0f, -1.0f),
      Vector3(1.0f,  1.0f,  1.0f),
      Vector3(1.0f,  1.0f,  1.0f),
      Vector3(-1.0f,  1.0f,  1.0f),
      Vector3(-1.0f,  1.0f, -1.0f),

      Vector3(-1.0f, -1.0f, -1.0f),
      Vector3(-1.0f, -1.0f,  1.0f),
      Vector3(1.0f, -1.0f, -1.0f),
      Vector3(1.0f, -1.0f, -1.0f),
      Vector3(-1.0f, -1.0f,  1.0f),
      Vector3(1.0f, -1.0f,  1.0f)
    };
    _skyBoxVertexData.reset(new VertexBuffer());
    _skyBoxVertexData->UploadData<Vector3>(skyBoxVertices, BufferUsage::Static);

    auto posLoc = static_cast<uint32>(VertexArribLocation::Position);

    glBindVertexArray(_skyBoxVertexData->_vaoId);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 12, nullptr);
    glEnableVertexAttribArray(posLoc);
    glBindVertexArray(0);
  }
  auto shader = _shaderCollection->GetShader("SkyBox.glsl");
  if (!shader)
  {
    return;
  }
  shader->SetMat4("view", scene->GetCamera()->GetViewMat());
  shader->SetMat4("projection", scene->GetCamera()->GetProjMat());
  shader->SetInt("skybox", 0);

  glDepthMask(GL_FALSE);
  shader->Bind();
  glBindVertexArray(_skyBoxVertexData->_vaoId);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, scene->GetSkyBox()->_id);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  glBindVertexArray(0);
  shader->Unbind();
  glDepthMask(GL_TRUE);

}

void Renderer::DrawTexturedObjects(ObjectPtrArray objects, const Vector3& ambientColour)
{
  auto shader = _shaderCollection->GetShader("Standard.shader");
  if (!shader)
  {
    return;
  }
  shader->Bind();
  shader->SetVec3("u_ambientColour", ambientColour);

  shader->BindUniformBlock(shader->GetUniformBlockIndex("Transforms"), static_cast<int32>(UniformBindingPoint::Transforms), _cameraBuffer->_uboId, _cameraBuffer->_sizeBytes);
  shader->BindUniformBlock(shader->GetUniformBlockIndex("PointLight"), static_cast<int32>(UniformBindingPoint::Light), _lightBuffer->_uboId, _lightBuffer->_sizeBytes);
  for (auto object : objects)
  {
    auto model = std::dynamic_pointer_cast<Model>(object->GetComponent("Model"));
    if (!model)
    {
      continue;
    }

    auto meshCount = model->GetMeshCount();
    for (size_t i = 0; i < meshCount; i++)
    {
      auto& staticMesh = model->GetMeshAtIndex(i);
      shader->SetMat4("u_model", object->GetTransform());

      auto material = staticMesh.GetMaterial();

//      if (material->HasTexture("DiffuseMap"))
//      {
//        auto diffuseMap = material->GetTexture("DiffuseMap");
//        glActiveTexture(GL_TEXTURE0);
//        diffuseMap->Bind();
//        shader->SetInt("material.diffuseMap", 0);
//      }
//
//      if (material->HasTexture("SpecularMap"))
//      {
//        auto bumpMap = material->GetTexture("SpecularMap");
//        glActiveTexture(GL_TEXTURE1);
//        bumpMap->Bind();
//        shader->SetInt("material.specularMap", 1);
//      }

      //if (material->HasTexture("NormalMap"))
      //{
      //  auto bumpMap = material->GetTexture("NormalMap");
      //  glActiveTexture(GL_TEXTURE2);
      //  bumpMap->Bind();
      //  shader->SetUniformInt("material.normalMap", 2);
      //}

      shader->SetFloat("u_material.SpecularExponent", material->GetSpecularExponent());
      shader->SetVec3("u_material.AmbientColour", material->GetAmbientColour());
      shader->SetVec3("u_material.DiffuseColour", material->GetDiffuseColour());
      shader->SetVec3("u_material.SpecularColour", material->GetSpecularColour());

      auto vertexData = staticMesh.GetVertexData();
      glBindVertexArray(vertexData->_vaoId);
      glDrawArrays(GL_TRIANGLES, 0, staticMesh._vertexCount);
      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
}

void Renderer::UploadCameraData(std::shared_ptr<OrbitalCamera> camera)
{
  if (_activeCamera != camera)
  {
    _activeCamera = camera;
    _projectionMatrixDirty = true;
    _viewMatrixDirty = true;
  }

  if (_cameraBuffer == nullptr)
  {
    _cameraBuffer.reset(new ConstantBuffer(144));
    _projectionMatrixDirty = true;
    _viewMatrixDirty = true;
  }

  if (_projectionMatrixDirty)
  {
    _cameraBuffer->UploadData(0, 64, &_activeCamera->GetProjMat()[0]);
  }

  if (_viewMatrixDirty)
  {
    _cameraBuffer->UploadData(64, 64, &_activeCamera->GetViewMat()[0]);
    _cameraBuffer->UploadData(128, 12, &_activeCamera->GetPos()[0]);
  }
}

void Renderer::UploadLightData(std::shared_ptr<WorldObject> lightObject)
{
  auto pointLight = std::dynamic_pointer_cast<PointLight>(lightObject->GetComponent("PointLight"));
  if (pointLight == nullptr)
  {
    return;
  }

  if (_lightBuffer == nullptr)
  {
    _lightBuffer.reset(new ConstantBuffer(80));
  }

  _lightBuffer->UploadData(0, 12, &pointLight->GetPosition()[0]);
  _lightBuffer->UploadData(16, 12, &pointLight->GetDiffuseColour()[0]);
  _lightBuffer->UploadData(32, 12, &pointLight->GetSpecularColour()[0]);

  float32 constContrib = pointLight->GetConstContrib();
  float32 linearContrib = pointLight->GetLinearContrib();
  float32 quadContrib = pointLight->GetQuadraticContrib();

  _lightBuffer->UploadData(44, 4, &constContrib);
  _lightBuffer->UploadData(48, 4, &linearContrib);
  _lightBuffer->UploadData(52, 4, &quadContrib);
}

void Renderer::ClearBuffer(ClearType clearType)
{
  switch (clearType)
  {
    case ClearType::Color:
      glClear(GL_COLOR_BUFFER_BIT);
      break;
    case ClearType::Depth:
      glClear(GL_DEPTH_BUFFER_BIT);
      break;
    case ClearType::All:
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      break;
  }
}
}

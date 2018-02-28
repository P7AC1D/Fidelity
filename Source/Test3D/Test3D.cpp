#include "Test3D.h"

#include "../Engine/Components/Component.h"
#include "../Engine/Geometry/MeshFactory.h"
#include "../Engine/Input/InputHandler.hpp"
#include "../Engine/Input/EventDispatcher.hpp"
#include "../Engine/Overlay/GuiCaption.hpp"
#include "../Engine/Overlay/GuiSystem.hpp"
#include "../Engine/Maths/Degree.hpp"
#include "../Engine/Maths/Math.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Radian.hpp"
#include "../Engine/Maths/Vector2.hpp"
#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Rendering/Material.h"
#include "../Engine/Rendering/Renderable.hpp"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/Rendering/Texture.hpp"
#include "../Engine/Utility/AssetManager.h"
#include "../Engine/Utility/FntLoader.hpp"
#include "../Engine/Utility/ObjLoader.hpp"
#include "../Engine/SceneManagement/Light.h"
#include "../Engine/SceneManagement/OrbitalCamera.h"
#include "../Engine/SceneManagement/SceneManager.h"
#include "../Engine/SceneManagement/Transform.h"
#include "../Engine/SceneManagement/WorldObject.h"

using namespace Rendering;
using namespace Utility;

float32 GetTerrainHeight(float32 x, float32 z)
{
  return Math::Sin(Radian((x * 2.0f * Math::Pi))) + Math::Sin(Radian((z * 3.0f * Math::Pi)));
}

float32 SampleHeight(const std::vector<Vector3>& data, const Vector3& scale, float32 x, float32 z, float32 eps = 0.04f)
{
  auto iter = std::find_if(data.begin(), data.end(), [&](const Vector3& vec)
  {
    return std::fabs(vec[0] * scale[0] - x) <= eps && std::fabs(vec[2] * scale[2] - z) <= eps;
  });
  if (iter != data.end())
  {
    return (*iter)[1] * scale[1];
  }
  return 0.0f;
}

Test3D::Test3D(const ApplicationDesc& desc):
  Application(desc)
{
}

void Test3D::OnStart()
{
  _sceneManager->SetAmbientLight(Vector3(0.25f, 0.25f, 0.20f));

  _camera = std::make_shared<OrbitalCamera>(0.0f, 90.0f, 2.0f);
  _camera->UpdateProjMat(GetWidth(), GetHeight(), 0.1f, 100.0f);
  _camera->SetPosition(Vector3(0.0f, 0.0f, 6.0f));
  _sceneManager->SetCamera(_camera);
    
  std::shared_ptr<SkyBox> skyBox(new SkyBox);
  skyBox->SetTexture(AssetManager::GetTextureCube("/Textures/SimpleSkyBox/", { "back.jpg", "bottom.jpg", "front.jpg", "left.jpg", "right.jpg", "top.jpg" }));
  _sceneManager->SetSkyBox(skyBox);
  
  /*Vector3 floorScale(100.0f);
  auto& floor = _sceneManager->CreateObject("floor");
  floor.SetScale(floorScale);
  auto plane = MeshFactory::CreatePlane(25);
  auto vertexData = plane->GetPositionVertexData();
  for (auto& vertex : vertexData)
  {
    vertex[1] = 0.05f * GetTerrainHeight(vertex[0], vertex[2]);
  }
  plane->SetPositionVertexData(vertexData);
  plane->GenerateNormals();

  auto& material = plane->GetMaterial();
  material.SetDiffuseColour(Colour(116, 244, 66));
  material.SetTexture("DiffuseMap", _assetManager->GetTexture("/Textures/TexturesCom_Grass0130_1_seamless_S.jpg"));
  std::shared_ptr<Renderable> planeModel(new Renderable);
  planeModel->PushMesh(*plane);
  floor.AttachRenderable(planeModel);

  for (float32 i = -40.0f; i < 40.0f; i += 4.0f)
  {
    for (float32 j = -40.0f; j < 40.0f; j += 4.0f)
    {
      auto treeModel = _sceneManager->LoadObjectFromFile("./../../Resources/Models/LowPolyTree/lowpolytree.obj");
      treeModel.GetTransform()->SetPosition(Vector3(i, SampleHeight(vertexData, floorScale, i, j), j));
    }
  }*/

  auto sphereANode = _sceneManager->CreateObject("sphereA");
  auto sphereA = MeshFactory::CreateIcosphere(3);
  auto sphereAMaterial = sphereA->GetMaterial();
  auto diffuseMap = AssetManager::GetTexture("/Textures/brick_floor_tileable_Base_Color.jpg");
  sphereA->GetMaterial()->SetTexture("DiffuseMap", diffuseMap);
  std::shared_ptr<Renderable> sphereModelA(new Renderable);
  sphereModelA->PushMesh(sphereA);
  sphereANode->AttachRenderable(sphereModelA);
  sphereANode->GetTransform()->Translate(Vector3(2.0f, 0.0f, 0.0f));

  auto sphereBNode = _sceneManager->CreateObject("sphereB");
  auto sphereB = MeshFactory::CreateIcosphere(3);
  auto sphereBMaterial = sphereB->GetMaterial();
  auto normalMap = AssetManager::GetTexture("/Textures/brick_floor_tileable_Normal.jpg");
  auto specularMap = AssetManager::GetTexture("/Textures/brick_floor_tileable_Glossiness.jpg");
  sphereBMaterial->SetTexture("DiffuseMap", diffuseMap);
  sphereBMaterial->SetTexture("NormalMap", normalMap);
  sphereBMaterial->SetTexture("SpecularMap", specularMap);
  std::shared_ptr<Renderable> sphereModelB(new Renderable);
  sphereModelB->PushMesh(sphereB);
  sphereBNode->AttachRenderable(sphereModelB);
  sphereBNode->GetTransform()->Translate(Vector3(-2.0f, 0.0f, 0.0f));

  //auto& cubeNode = _sceneManager->CreateObject("cube");
  //auto cubeMesh = MeshFactory::CreateCube();
  //auto& cubeMaterial = cubeMesh->GetMaterial();
  //auto diffuseMap = _assetManager->GetTexture("/Textures/bricks2.jpg");
  //auto normalMap = _assetManager->GetTexture("/Textures/bricks2_normal.jpg");
  //auto depthMap = _assetManager->GetTexture("/Textures/bricks2_disp.jpg");
  //cubeMaterial.SetTexture("DiffuseMap", diffuseMap);
  //cubeMaterial.SetTexture("NormalMap", normalMap);
  //cubeMaterial.SetTexture("DepthMap", depthMap);
  //std::shared_ptr<Renderable> cubeModel(new Renderable);
  //cubeModel->PushMesh(*cubeMesh);
  //cubeNode.AttachRenderable(cubeModel);

  _light = &_sceneManager->CreateLight(LightType::Directional);
  _light->SetColour(Colour(255, 240, 170));

  _inputHandler->BindButtonToState("ActivateCameraLook", Button::Button_RMouse);
  _inputHandler->BindAxisToState("CameraZoom", Axis::MouseScrollXY);
  _inputHandler->BindAxisToState("CameraLook", Axis::MouseXY);

  _eventDispatcher->Register("CameraZoom", [&](const InputEvent& inputEvent, int32 dt)
  {
    _camera->Zoom(dt * inputEvent.AxisPosDelta[1] * 0.1f);
  });
  _eventDispatcher->Register("CameraLook", [&](const InputEvent& inputEvent, int32 dt)
  {
    if (_inputHandler->IsButtonStateActive("ActivateCameraLook"))
    {
      float32 sensitivity = 0.1f;
      float32 yaw = Radian(static_cast<float32>(inputEvent.AxisPosDelta[0]) * sensitivity * dt).InRadians();
      float32 pitch = Radian(static_cast<float32>(inputEvent.AxisPosDelta[1]) * sensitivity * dt).InRadians();
      _camera->Rotate(yaw, pitch);
    }
  });
  
  GuiCaptionDesc guiCaptionDesc;
  guiCaptionDesc.FontColour = Colour::Black;
  guiCaptionDesc.Name = "Test Caption";
  guiCaptionDesc.Text = "Test Caption";
  guiCaptionDesc.FontSize = 20;
  guiCaptionDesc.MaxWidth = 100;
  guiCaptionDesc.Font = "GillSans";
  auto caption = GuiSystem::Get()->CreateCaption(guiCaptionDesc);
  
  GuiPanelDesc guiPanelDesc;
  guiPanelDesc.Name = "Test panel";
  guiPanelDesc.Colour = Colour(0, 0, 0, 150);
  guiPanelDesc.Right = 150;
  guiPanelDesc.Bottom = GetHeight();
  _testPanel = GuiSystem::Get()->CreatePanel(guiPanelDesc);

  GuiPanelDesc guiChildPanelDesc;
  guiChildPanelDesc.Name = "Test child panel";
  guiChildPanelDesc.Colour = Colour(0, 0, 255, 150);
  guiChildPanelDesc.Left = 50;
  guiChildPanelDesc.Right = 100;
  guiChildPanelDesc.Top = 50;
  guiChildPanelDesc.Bottom = 100;
  _testChildPanel = GuiSystem::Get()->CreatePanel(guiChildPanelDesc);
  _testChildPanel->SetParent(_testPanel);
  caption->SetParent(_testPanel);
  _testChildPanel->SetOnMouseEnter([&]()
  {
    _testChildPanel->SetColour(Colour(255, 0, 0, 150));
  });
  _testChildPanel->SetOnMouseLeave([&]()
  {
    _testChildPanel->SetColour(Colour(0, 0, 255, 150));
  });}

void Test3D::OnUpdate(uint32 dtMs)
{
  static float32 delta = 0.0f;
  auto direction = _light->GetDirection();
  delta += dtMs * 0.0001f;
  direction[0] = Math::Cos(Radian(delta));
  direction[1] = -1.0f;
  direction[2] = Math::Cos(Radian(delta + delta));
  _light->SetDirection(Vector3::Normalize(direction));

  //_onScreenFpsCounter->UpdateText(std::to_string(GetAverageFps(dtMs)) + " FPS " + std::to_string(GetAverageTickMs(dtMs)) + " ms");
}

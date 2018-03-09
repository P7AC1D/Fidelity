#include "Test3D.h"

#include <iomanip>
#include <sstream>

#include "../Engine/Components/Component.h"
#include "../Engine/Geometry/MeshFactory.h"
#include "../Engine/Input/InputHandler.hpp"
#include "../Engine/Input/EventDispatcher.hpp"
#include "../Engine/Overlay/GuiCaption.hpp"
#include "../Engine/Overlay/GuiCheckBox.hpp"
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

  _camera.reset(new OrbitalCamera());
  _camera->SetPerspective(Degree(67.67f), GetWidth(), GetHeight(), 0.1f, 1000.0f);
  _camera->LookAt(Vector3(0.0f, 0.0f, 5.0f), Vector3(0.0f, 0.0f, 0.0f));
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

 /* auto object = _sceneManager->LoadObjectFromFile("Models/Sponza/sponza.obj");
  object->GetTransform()->SetScale(Vector3(0.1f));*/

  _light = &_sceneManager->CreateLight(LightType::Directional);
  _light->SetColour(Colour(255, 240, 170));

  _inputHandler->BindButtonToState("ActivateCameraLook", Button::Button_RMouse);
  _inputHandler->BindAxisToState("CameraZoom", Axis::MouseScrollXY);
  _inputHandler->BindAxisToState("CameraLook", Axis::MouseXY);

  _eventDispatcher->Register("CameraZoom", [&](const InputEvent& inputEvent, int32 dt)
  {
    _camera->Zoom(static_cast<float32>(inputEvent.AxisPosDelta[1]), dt);
  });
  _eventDispatcher->Register("CameraLook", [&](const InputEvent& inputEvent, int32 dt)
  {
    if (_inputHandler->IsButtonStateActive("ActivateCameraLook"))
    {
      Radian yaw(static_cast<float32>(-inputEvent.AxisPosDelta[1]));
      Radian pitch(static_cast<float32>(-inputEvent.AxisPosDelta[0]));
      _camera->RotateAboutTarget(yaw, pitch, dt);
    }
  });
  
  GuiCaptionDesc guiCaptionDesc;
  guiCaptionDesc.FontColour = Colour::Black;
  guiCaptionDesc.Name = "Test Caption";
  guiCaptionDesc.Text = "Test Caption";
  guiCaptionDesc.FontSize = 20;
  guiCaptionDesc.MaxWidth = 100;
  guiCaptionDesc.Font = "GillSans";
  _fpsCounter = GuiSystem::Get()->CreateCaption(guiCaptionDesc);
  
  GuiPanelDesc guiPanelDesc;
  guiPanelDesc.Name = "Test panel";
  guiPanelDesc.Colour = Colour(0, 0, 0, 200);
  guiPanelDesc.Left = GetWidth() - 150;
  guiPanelDesc.Right = GetWidth();
  guiPanelDesc.Bottom = GetHeight();
  _testPanel = GuiSystem::Get()->CreatePanel(guiPanelDesc);

  GuiCheckBoxDesc guiCheckBoxDesc;
  guiCheckBoxDesc.Name = "Test Checkbox";
  guiCheckBoxDesc.Position.X = 10;
  guiCheckBoxDesc.Position.Y = 10;
  guiCheckBoxDesc.Size = 15;
  guiCheckBoxDesc.Colour = Colour::White;
  auto testCheckBox = GuiSystem::Get()->CreateCheckBox(guiCheckBoxDesc);
  auto checkedTexture = AssetManager::GetTexture("/Textures/unchecked-checkbox.png");
  testCheckBox->SetUncheckedTexture(AssetManager::GetTexture("/Textures/unchecked-checkbox.png"));
  testCheckBox->SetCheckedTexture(AssetManager::GetTexture("/Textures/checked-checkbox.png"));
  testCheckBox->SetParent(_testPanel);

  GuiCaptionDesc checkboxLabelDesc;
  checkboxLabelDesc.FontColour = Colour::White;
  checkboxLabelDesc.Name = "Checkbox Label";
  checkboxLabelDesc.Text = "Checkbox Label";
  checkboxLabelDesc.FontSize = 13;
  checkboxLabelDesc.MaxWidth = 100;
  checkboxLabelDesc.Font = "GillSans";
  auto checkboxLabel = GuiSystem::Get()->CreateCaption(checkboxLabelDesc);
  checkboxLabel->SetParent(_testPanel);
  checkboxLabel->SetBounds(BoundingBox(30, 100, 10, 25));
}

void Test3D::OnUpdate(uint32 dtMs)
{
  static float32 delta = 0.0f;
  auto direction = _light->GetDirection();
  delta += dtMs * 0.0001f;
  direction[0] = Math::Cos(Radian(delta));
  direction[1] = -1.0f;
  direction[2] = Math::Cos(Radian(delta + delta));
  _light->SetDirection(Vector3::Normalize(direction));

  std::stringstream ss;
  ss.precision(4);
  ss << GetAverageFps(dtMs) << " FPS " << GetAverageTickMs(dtMs) << " ms";
  _fpsCounter->SetText(ss.str());
}

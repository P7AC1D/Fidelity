#include "Test3D.h"

#include "../Engine/Components/Component.h"
#include "../Engine/Components/PointLight.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Radian.hpp"
#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Rendering/Material.h"
#include "../Engine/Rendering/MeshFactory.h"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/UI/Panel.h"
#include "../Engine/UI/UIManager.h"
#include "../Engine/Utility/AssetManager.h"
#include "../Engine/SceneManagement/OrbitalCamera.h"
#include "../Engine/SceneManagement/Scene.h"
#include "../Engine/SceneManagement/SceneManager.h"
#include "../Engine/SceneManagement/SceneNode.h"
#include "../Engine/SceneManagement/WorldObject.h"

using namespace Components;
using namespace Rendering;
using namespace Platform;
using namespace UI;
using namespace Utility;
using namespace SceneManagement;

Test3D::Test3D(const ApplicationDesc& desc):
  Application(desc)
{
}

void Test3D::OnStart()
{
  auto scene = _sceneManager->GetActiveScene();
  auto rootNode = scene->GetRootNode();
  _sceneNode = rootNode->CreateChild("child");

  scene->SetAmbientLight(Vector3(0.1f, 0.1f, 0.1f));

  _camera = std::make_shared<OrbitalCamera>(0.0f, 90.0f, 2.0f);
  _camera->UpdateProjMat(GetWidth(), GetHeight(), 0.1f, 100.0f);
  _camera->SetPosition(Vector3(0.0f, 0.0f, 6.0f));
  scene->SetCamera(_camera);

  auto cube = _sceneNode->CreateObject("cube");
  auto cubeMesh = MeshFactory::CreateCube();
  auto cubeMaterial = cubeMesh->GetMaterial();
  cubeMaterial->SetTexture("DiffuseMap", _assetManager->GetTexture("container2.png", true));
  cubeMaterial->SetTexture("BumpMap", _assetManager->GetTexture("container2_specular.png"));
  cube->AddComponent(cubeMesh);
  
  _object = _sceneNode->CreateObject("cube2");
  _object->SetPosition(Vector3(2.0f, 1.0f, 0.0f));
  _object->AddComponent(cubeMesh);

  auto floor = rootNode->CreateObject("floor");
  auto floorMesh = MeshFactory::CreateCube();
  floor->SetScale(Vector3(20.0f, 0.01f, 20.0f));
  floor->SetPosition(Vector3(0.0f, -2.0f, 0.0f));
  floorMesh->GetMaterial()->SetTexture("DiffuseMap", _assetManager->GetTexture("brick_floor_tileable_Base_Color.jpg", true));
  floorMesh->GetMaterial()->SetTexture("BumpMap", _assetManager->GetTexture("brick_floor_tileable_Glossiness.jpg"));
  floor->AddComponent(floorMesh);

  auto lightA = std::make_shared<WorldObject>("lightA");
  auto lightB = std::make_shared<WorldObject>("lightB");
  auto lightC = std::make_shared<WorldObject>("lightC");

  auto lightComponentA = std::make_shared<PointLight>(Vector3(0.0f, 2.0f, 0.0f),
                                                      Vector3(0.8f, 0.0f, 0.0f),
                                                      Vector3(0.8f, 0.0f, 0.0f),
                                                      1.0f, 
                                                      0.045f,
                                                      0.0075f);
  auto lightComponentB = std::make_shared<PointLight>(Vector3(-4.0f, 4.0f, -4.0f),
                                                      Vector3(0.0f, 0.8f, 0.0f),
                                                      Vector3(0.0f, 0.8f, 0.0f),
                                                      1.0f, 
                                                      0.045f,
                                                      0.0075f);
  auto lightComponentC = std::make_shared<PointLight>(Vector3(4.0f, 4.0f, -4.0f),
                                                      Vector3(0.0f, 0.0f, 0.8f),
                                                      Vector3(0.0f, 0.0f, 0.8f),
                                                      1.0f, 
                                                      0.045f,
                                                      0.0075f);
  lightA->AddComponent(lightComponentA);
  lightB->AddComponent(lightComponentB);
  lightC->AddComponent(lightComponentC);

  rootNode->AddObject(lightA);
  rootNode->AddObject(lightB);
  rootNode->AddObject(lightC);

  auto skyBox = _assetManager->GetCubeMap(std::vector<std::string>{ "/CubeMaps/right.jpg",
                                                                    "/CubeMaps/left.jpg",
                                                                    "/CubeMaps/top.jpg",
                                                                    "/CubeMaps/bottom.jpg",
                                                                    "/CubeMaps/back.jpg",
                                                                    "/CubeMaps/front.jpg" });
  scene->SetSkyBox(skyBox);

  //auto panel1 = std::make_shared<Panel>("OSD1", 300, 300, GetWindowWidth() - 310, 10);
  //panel1->SetTexture(_assetManager->GetTexture("crate0_normal.png"));
  //auto panel2 = std::make_shared<Panel>("OSD2", 300, 300, 10, 10);
  //panel2->SetTexture(_assetManager->GetTexture("crate0_normal.png"));
  //_uiManager->AttachPanel(panel1);
  //_uiManager->AttachPanel(panel2);
}

void Test3D::OnInput()
{
  //if (_inputManager->IsKeyDown(GLFW_KEY_D))
  //{
  //  _camera->Pan(0.1f, 0.0f);
  //}
  //else if (_inputManager->IsKeyDown(GLFW_KEY_A))
  //{
  //  _camera->Pan(-0.1f, 0.0f);
  //}

  //auto mouseScrollOffset = _inputManager->GetMouseScrollOffset();
  //if (mouseScrollOffset != 0)
  //{
  //  _camera->Zoom(mouseScrollOffset);
  //  _inputManager->SetMouseScrollOffset(0.0f);
  //}

  //static float64 prevXPos;
  //static float64 prevYPos;
  //auto mousePosition = _inputManager->GetMousePosition();
  //if (_inputManager->IsButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
  //{      
  //  float32 yaw = Radian(static_cast<float32>(prevXPos - mousePosition.XPosition)* 0.01f).InRadians();
  //  float32 pitch = Radian(static_cast<float32>(prevYPos - mousePosition.YPosition) * 0.01f).InRadians();
  //  _camera->Rotate(yaw, pitch);    
  //}
  //prevXPos = mousePosition.XPosition;
  //prevYPos = mousePosition.YPosition;
}

void Test3D::OnTick(uint32 dtMs)
{
  _object->Rotate(Quaternion(Vector3(0.0f, 1.0f, 1.0f), Radian(0.0005f * dtMs)));
  _sceneNode->Rotate(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(0.0005f * dtMs)));
}
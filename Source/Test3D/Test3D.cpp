#include "Test3D.h"

#include "../Engine/Components/Component.h"
#include "../Engine/Components/PointLight.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Input/InputHandler.hpp"
#include "../Engine/Input/EventDispatcher.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Radian.hpp"
#include "../Engine/Maths/Vector2.hpp"
#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Rendering/Material.h"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/UI/Panel.h"
#include "../Engine/UI/UIManager.h"
#include "../Engine/Utility/AssetManager.h"
#include "../Engine/Utility/MeshFactory.h"
#include "../Engine/Utility/ObjLoader.hpp"
#include "../Engine/SceneManagement/OrbitalCamera.h"
#include "../Engine/SceneManagement/Scene.h"
#include "../Engine/SceneManagement/SceneManager.h"
#include "../Engine/SceneManagement/SceneNode.h"
#include "../Engine/SceneManagement/WorldObject.h"

using namespace Components;
using namespace Rendering;
using namespace UI;
using namespace Utility;
using namespace SceneManagement;

Test3D::Test3D(const ApplicationDesc& desc):
  Application(desc),
  _rotatingCamera(false)
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

  //auto cube = _sceneNode->CreateObject("cube");
  //cube->SetScale(Vector3(0.5f, 0.5f, 0.5f));
  //cube->SetPosition(Vector3(2.0f, 2.0f, 2.0f));
  //auto cubeModel = ObjLoader::LoadFromFile("./../../Assets/Models/Container/", "container.obj", *_assetManager);
  //auto cubeMaterial = cubeModel->GetMeshAtIndex(0).GetMaterial();
  //cubeMaterial->SetTexture("DiffuseMap", _assetManager->GetTexture("Textures/crate0_diffuse.png", true));
  //cubeMaterial->SetTexture("SpecularMap", _assetManager->GetTexture("Textures/crate0_bump.png"));
  //cubeMaterial->SetTexture("NormalMap", _assetManager->GetTexture("Textures/crate0_normal.png"));
  //cube->AddComponent(cubeModel);

  //_object = _sceneNode->CreateObject("cube2");
  //_object->SetScale(Vector3(0.5f, 0.5f, 0.5f));
  //_object->SetPosition(Vector3(2.0f, 1.0f, 0.0f));
  //_object->AddComponent(cubeModel);

  auto treeModel = ObjLoader::LoadFromFile("./../../Assets/Models/LowPolyTree/", "lowpolotree_triangulated.obj", *_assetManager);  
  auto tree = _sceneNode->CreateObject("tree");
  tree->AddComponent(treeModel);
  
  auto floor = _sceneNode->CreateObject("floor");
  floor->SetScale(Vector3(10.0f));
  floor->SetPosition(Vector3(0.0f, -2.0f, 0.0f));
  auto plane = MeshFactory::CreatePlane(10);
  auto material = plane->GetMaterial();
  material->SetAmbientColour(Vector3(0.0f));
  material->SetDiffuseColour(Vector3(0.7f));
  material->SetSpecularColour(Vector3(0.25f));
  material->SetSpecularShininess(1.0f);
  material->SetTexture("DiffuseMap", _assetManager->GetTexture("/Textures/177.JPG"));
  std::shared_ptr<Model> planeModel(new Model);
  planeModel->PushMesh(*plane);
  floor->AddComponent(planeModel);

 /* auto floor = rootNode->CreateObject("floor");
  auto floorModel = _assetManager->GetModel("Models/Container/container.obj");
  floor->SetScale(Vector3(20.0f, 0.01f, 20.0f));
  floor->SetPosition(Vector3(0.0f, -2.0f, 0.0f));
  floorModel->GetMeshAtIndex(0).GetMaterial()->SetTexture("DiffuseMap", _assetManager->GetTexture("Textures/brick_floor_tileable_Base_Color.jpg", true));
  floorModel->GetMeshAtIndex(0).GetMaterial()->SetTexture("BumpMap", _assetManager->GetTexture("Textures/brick_floor_tileable_Glossiness.jpg"));
  floor->AddComponent(floorModel);*/

  auto lightA = std::make_shared<WorldObject>("lightA");
  auto lightComponentA = std::make_shared<PointLight>(Vector3(3.0f, 3.0f, 3.0f),
                                                      Vector3(1.0f, 1.0f, 1.0f),
                                                      Vector3(0.2f, 0.2f, 0.2f),
                                                      1.0f, 
                                                      0.0f,
                                                      1.0f);
  lightA->AddComponent(lightComponentA);
  rootNode->AddObject(lightA);

  _inputHandler->BindButtonToState("ActivateCameraLook", Button::Button_LMouse);
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
}

void Test3D::OnUpdate(uint32 dtMs)
{
  //_object->Rotate(Quaternion(Vector3(0.0f, 1.0f, 1.0f), Radian(0.0005f * dtMs)));
  //_sceneNode->Rotate(Quaternion(Vector3(0.0f, 1.0f, 0.0f), Radian(0.0005f * dtMs)));
}

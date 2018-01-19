#include "Test3D.h"

#include "../Engine/Components/Component.h"
#include "../Engine/Components/PointLight.h"
#include "../Engine/Input/InputHandler.hpp"
#include "../Engine/Input/EventDispatcher.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Radian.hpp"
#include "../Engine/Maths/Vector2.hpp"
#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Rendering/Material.h"
#include "../Engine/Rendering/Renderable.hpp"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/UI/Panel.h"
#include "../Engine/UI/UIManager.h"
#include "../Engine/Utility/AssetManager.h"
#include "../Engine/Utility/MeshFactory.h"
#include "../Engine/Utility/ObjLoader.hpp"
#include "../Engine/SceneManagement/OrbitalCamera.h"
#include "../Engine/SceneManagement/SceneManager.h"
#include "../Engine/SceneManagement/Transform.h"
#include "../Engine/SceneManagement/WorldObject.h"

using namespace Components;
using namespace Rendering;
using namespace UI;
using namespace Utility;

Test3D::Test3D(const ApplicationDesc& desc):
  Application(desc),
  _rotatingCamera(false)
{
}

void Test3D::OnStart()
{
  _sceneManager->SetAmbientColour(Vector3(0.1f, 0.1f, 0.1f));

  _camera = std::make_shared<OrbitalCamera>(0.0f, 90.0f, 2.0f);
  _camera->UpdateProjMat(GetWidth(), GetHeight(), 0.1f, 100.0f);
  _camera->SetPosition(Vector3(0.0f, 0.0f, 6.0f));
  _sceneManager->SetCamera(_camera);

  auto& tree = _sceneManager->LoadObjectFromFile("./../../Assets/Models/LowPolyTree/lowpolotree_triangulated.obj");
  
  auto& floor = _sceneManager->CreateObject("floor");
  floor.SetScale(Vector3(10.0f));
  floor.SetPosition(Vector3(0.0f, -2.0f, 0.0f));
  auto& plane = MeshFactory::CreatePlane(10);
  auto& material = plane->GetMaterial();
  material->SetAmbientColour(Vector3(0.0f));
  material->SetDiffuseColour(Vector3(0.7f));
  material->SetSpecularColour(Vector3(0.25f));
  material->SetSpecularShininess(1.0f);
  material->SetTexture("DiffuseMap", _assetManager->GetTexture("/Textures/177.JPG"));
  std::shared_ptr<Renderable> planeModel(new Renderable);
  planeModel->PushMesh(*plane);
  floor.SetRenderable(planeModel);

 /* auto floor = rootNode->CreateObject("floor");
  auto floorModel = _assetManager->GetModel("Models/Container/container.obj");
  floor->SetScale(Vector3(20.0f, 0.01f, 20.0f));
  floor->SetPosition(Vector3(0.0f, -2.0f, 0.0f));
  floorModel->GetMeshAtIndex(0).GetMaterial()->SetTexture("DiffuseMap", _assetManager->GetTexture("Textures/brick_floor_tileable_Base_Color.jpg", true));
  floorModel->GetMeshAtIndex(0).GetMaterial()->SetTexture("BumpMap", _assetManager->GetTexture("Textures/brick_floor_tileable_Glossiness.jpg"));
  floor->AddComponent(floorModel);*/

  auto& light = _sceneManager->CreateObject("light");
  light.AddComponent(std::make_shared<PointLight>(
    Vector3(3.0f, 3.0f, 3.0f),
    Vector3(1.0f, 1.0f, 1.0f),
    7.0f));

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

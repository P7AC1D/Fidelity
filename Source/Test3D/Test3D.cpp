#include "Test3D.h"

#include <iomanip>
#include <sstream>

#include "../Engine/Geometry/MeshFactory.h"
#include "../Engine/Input/InputHandler.hpp"
#include "../Engine/Input/EventDispatcher.hpp"
#include "../Engine/Maths/Degree.hpp"
#include "../Engine/Maths/Math.hpp"
#include "../Engine/Maths/Quaternion.hpp"
#include "../Engine/Maths/Radian.hpp"
#include "../Engine/Maths/Vector2.hpp"
#include "../Engine/Maths/Vector3.hpp"
#include "../Engine/Utility/TextureLoader.hpp"
#include "../Engine/Rendering/Material.hpp"
#include "../Engine/Rendering/Renderable.hpp"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/RenderApi/Texture.hpp"
#include "../Engine/SceneManagement/Component.hpp"
#include "../Engine/SceneManagement/Light.h"
#include "../Engine/SceneManagement/Actor.hpp"
#include "../Engine/SceneManagement/FpsCamera.hpp"
#include "../Engine/SceneManagement/OrbitalCamera.hpp"
#include "../Engine/SceneManagement/SceneManager.h"
#include "../Engine/SceneManagement/SceneNode.hpp"
#include "../Engine/SceneManagement/Transform.h"

Test3D::Test3D(const ApplicationDesc& desc):
  Application(desc)
{
}

void Test3D::OnStart()
{
  _camera.reset(new OrbitalCamera(1.0f, 10.0f));
  _camera->SetPerspective(Degree(67.67f), GetWidth(), GetHeight(), 0.1f, 10000.0f);
  _camera->LookAt(Vector3(0.0f, 0.0f, 45.0f), Vector3::Zero);
	SceneManager::Get()->SetCamera(_camera);

	auto directionalLight = SceneManager::Get()->CreateLight(LightType::Directional, "DirectionLight");
	SceneManager::Get()->SetDirectionLight(directionalLight);
  
  //auto model = SceneManager::Get()->LoadModelFromFile("./../../Resources/Models/living_room/living_room.obj", true);
  
//  auto rootSceneNode = SceneManager::Get()->GetRootSceneNode();
//  for (int32 i = -30; i < 30; i += 3)
//  {
//    for (int32 j = -30; j < 30; j += 3)
//    {
//      for (int32 k = -30; k < 30; k += 3)
//      {
//        auto actor = rootSceneNode->CreateActor("test_model");
//        auto renderable = actor->CreateComponent<Renderable>();
//        auto mesh = MeshFactory::CreateCube();
//        renderable->SetMesh(mesh);
//        actor->GetTransform()->Translate(Vector3(i, j, k));
//      }
//    }
//  }

	//auto rootSceneNode = SceneManager::Get()->GetRootSceneNode();
	//auto actor = rootSceneNode->CreateActor("test_model");
	//auto renderable = actor->CreateComponent<Renderable>();
	//auto mesh = MeshFactory::CreateCube();
	//renderable->SetMesh(mesh);
	//actor->GetTransform()->Translate(Vector3(0, 0, 20.0f));

  auto rootSceneNode = SceneManager::Get()->GetRootSceneNode();
  auto actor = rootSceneNode->CreateActor("test_model");
  auto renderable = actor->CreateComponent<Renderable>();
  auto mesh = MeshFactory::CreateCube();
  renderable->SetMesh(mesh);
  auto material = mesh->GetMaterial();
  material->SetSpecularExponent(4.0f);
  auto diffuseTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Base_Color.jpg", true);
  auto normalTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Normal.jpg");
  auto specularTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Glossiness.jpg");
  material->SetDiffuseTexture(diffuseTexture);
  material->SetNormalTexture(normalTexture);
  material->SetSpecularTexture(specularTexture);

	auto floor = rootSceneNode->CreateActor("floor");
	floor->SetPosition(Vector3(0.0f, -10.0f, 0.0f));
	floor->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	auto renderableFloor = floor->CreateComponent<Renderable>();
	auto plane = MeshFactory::CreatePlane();
	renderableFloor->SetMesh(plane);
	plane->GetMaterial()->SetDiffuseTexture(TextureLoader::LoadFromFile2D("./../../Resources/Textures/177.JPG", true));
	
  _inputHandler->BindButtonToState("ActivateCameraLook", Button::Button_LMouse);
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
}

void Test3D::OnUpdate(uint32 dtMs)
{
  std::stringstream ss;
  ss.precision(4);
  ss << GetAverageFps(dtMs) << " FPS " << GetAverageTickMs(dtMs) << " ms";
}

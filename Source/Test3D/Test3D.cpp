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
  _camera.reset(new OrbitalCamera(5.0f, 10.0f));
  _camera->SetPerspective(Degree(67.67f), GetWidth(), GetHeight(), 0.1f, 1000.0f);
  _camera->LookAt(Vector3(4.0f, 4.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f));
  _sceneManager->SetCamera(_camera);

	auto directionalLight = _sceneManager->CreateLight(LightType::Directional, "DirectionLight");
	_sceneManager->SetDirectionLight(directionalLight);
  
//  _sceneManager->LoadModelFromFile("./../../Resources/Models/Sponza/sponza.obj");
  
  auto rootSceneNode = _sceneManager->GetRootSceneNode();
  for (int32 i = -30; i < 30; i += 3)
  {
    for (int32 j = -30; j < 30; j += 3)
    {
//      for (int32 k = -30; k < 30; k += 3)
      {
        auto actor = rootSceneNode->CreateActor("test_model");
        auto renderable = actor->CreateComponent<Renderable>();
        auto mesh = MeshFactory::CreateCube();
        renderable->SetMesh(mesh);
        actor->GetTransform()->Translate(Vector3(i, j, 0));
      }
    }
  }
//  auto actor = rootSceneNode->CreateActor("test_model");
//  auto renderable = actor->CreateComponent<Renderable>();
//  auto mesh = MeshFactory::CreateCube();
//  renderable->SetMesh(mesh);
//  auto material = mesh->GetMaterial();
//  material->SetSpecularShininess(4.0f);
//  auto diffuseTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Base_Color.jpg", true);
//  auto normalTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Normal.jpg");
//  auto specularTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Glossiness.jpg");
//  material->SetTexture("DiffuseMap", diffuseTexture);
//  material->SetTexture("NormalMap", normalTexture);
//  material->SetTexture("SpecularMap", specularTexture);
	
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
}

void Test3D::OnUpdate(uint32 dtMs)
{
  std::stringstream ss;
  ss.precision(4);
  ss << GetAverageFps(dtMs) << " FPS " << GetAverageTickMs(dtMs) << " ms";
}

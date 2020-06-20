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
#include "../Engine/SceneManagement/ActorNode.hpp"
#include "../Engine/SceneManagement/CameraNode.hpp"
#include "../Engine/SceneManagement/GenericNode.hpp"
#include "../Engine/SceneManagement/SceneNode.hpp"
#include "../Engine/SceneManagement/Transform.h"
#include "../Engine/Utility/ModelLoader.hpp"

static const float32 CAMERA_ROTATION_FACTOR = 0.005f;
static const float32 CAMERA_ZOOM_FACTOR = 0.01f;

Test3D::Test3D(const ApplicationDesc& desc):
  Application(desc),
  _cameraTarget(Vector3::Zero)
{
}

void Test3D::OnStart()
{
  _camera = SceneNode::Create<CameraNode>();
  _camera->SetPerspective(Degree(67.67f), GetWidth(), GetHeight(), 0.1f, 10000.0f);
  _camera->GetTransform().LookAt(Vector3(0.0f, 0.0f, 20.0f), _cameraTarget);
	
  _sceneGraph->AddChild(_camera);

  auto light = SceneNode::Create<LightNode>();
  light->SetColour(Colour(228, 179, 74));
  light->SetIntensity(0.7f);
  light->GetTransform().SetRotation(Quaternion(Degree(-120.0f), 30.0f, 6.0f));
  _sceneGraph->AddChild(light);

	//auto directionalLight = SceneManager::Get()->CreateLight(LightType::Directional, "DirectionLight");
 // directionalLight->SetDirection(Vector3(0.1f, -1.0f, 0.1f));
	//SceneManager::Get()->SetDirectionLight(directionalLight);
    
  auto sponza(ModelLoader::LoadFromFile("./../../Resources/Models/Sponza/sponza.obj", false));
  _sceneGraph->AddChild(sponza);
	
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

	
  
  //auto actor = SceneNode::Create<ActorNode>();
  //_sceneGraph->AddChild(actor);
  //auto renderable = ActorNode::CreateComponent<Renderable>();
  //actor->AddComponent<Renderable>(renderable);
  //auto mesh = MeshFactory::CreateCube();
  //renderable->SetMesh(mesh);
  //auto material = mesh->GetMaterial();
  //material->SetSpecularExponent(4.0f);
  //auto diffuseTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Base_Color.jpg", true);
  //auto normalTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Normal.jpg");
  //auto specularTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/brick_floor_tileable_Glossiness.jpg");
  //material->SetDiffuseTexture(diffuseTexture);
  //material->SetNormalTexture(normalTexture);
  //material->SetSpecularTexture(specularTexture);
//
//  auto floor = rootSceneNode->CreateActor("floor");
//  floor->SetPosition(Vector3(0.0f, -10.0f, 0.0f));
//  floor->SetScale(Vector3(10.0f, 10.0f, 10.0f));
//  auto renderableFloor = floor->CreateComponent<Renderable>();
//  auto plane = MeshFactory::CreatePlane();
//  renderableFloor->SetMesh(plane);
//  plane->GetMaterial()->SetDiffuseTexture(TextureLoader::LoadFromFile2D("./../../Resources/Textures/177.JPG", true));
	
  _inputHandler->BindButtonToState("ActivateCameraLook", Button::Button_LMouse);
	_inputHandler->BindAxisToState("CameraZoom", Axis::MouseScrollXY);
  _inputHandler->BindAxisToState("CameraLook", Axis::MouseXY);

	_eventDispatcher->Register("CameraZoom", [&](const InputEvent& inputEvent, int32 dt)
	{
		ZoomCamera(static_cast<float32>(inputEvent.AxisPosDelta[1]), dt);
	});

  _eventDispatcher->Register("CameraLook", [&](const InputEvent& inputEvent, int32 dt)
  {
    if (_inputHandler->IsButtonStateActive("ActivateCameraLook"))
    {
      Radian yaw(static_cast<float32>(-inputEvent.AxisPosDelta[1]));
      Radian pitch(static_cast<float32>(-inputEvent.AxisPosDelta[0]));
      RotateCamera(yaw, pitch, dt);
    }
  });
}

void Test3D::OnUpdate(uint32 dtMs)
{
  std::stringstream ss;
  ss.precision(4);
  ss << GetAverageFps(dtMs) << " FPS " << GetAverageTickMs(dtMs) << " ms";
}

void Test3D::RotateCamera(const Degree& deltaX, const Degree& deltaY, int32 dtMs) const
{
  Transform& cameraTransform = _camera->GetTransform();	
  float32 velocity(CAMERA_ROTATION_FACTOR * static_cast<float32>(dtMs));
  Quaternion pitch(cameraTransform.GetRight(), velocity * deltaX.InRadians());
  Quaternion yaw(cameraTransform.GetUp(), velocity * deltaY.InRadians());
  Quaternion rotation(pitch * yaw);
  Vector3 newPosition(rotation.Rotate(cameraTransform.GetPosition()));
  cameraTransform.LookAt(newPosition, _cameraTarget);
}

void Test3D::ZoomCamera(float32 delta, int32 dtMs) const
{
  Transform& cameraTransform = _camera->GetTransform();
  Vector3 cameraForward = cameraTransform.GetForward();
  Vector3 cameraPostion = cameraTransform.GetPosition();
	
  float32 distanceToTarget = Vector3::Length(cameraPostion - _cameraTarget);
  float32 velocity(CAMERA_ZOOM_FACTOR * distanceToTarget * static_cast<float32>(dtMs));
  Vector3 newPosition(cameraPostion + cameraForward * velocity * -delta);

  Vector3 cameraToOldPos(cameraPostion - _cameraTarget);
  Vector3 cameraToNewPos(newPosition - _cameraTarget);
  if (Vector3::Dot(cameraToNewPos, cameraToOldPos) < 0.0f)
  {
    newPosition = _cameraTarget + cameraForward * 0.1f;
  }
  cameraTransform.LookAt(newPosition, _cameraTarget);
}
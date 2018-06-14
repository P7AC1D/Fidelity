#include "Test3D.h"

#include <iomanip>
#include <sstream>

#include "../Engine/Components/Component.h"
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
#include "../Engine/Rendering/Material.h"
#include "../Engine/Rendering/Renderable.hpp"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/SceneManagement/Light.h"
#include "../Engine/SceneManagement/FpsCamera.hpp"
#include "../Engine/SceneManagement/OrbitalCamera.hpp"
#include "../Engine/SceneManagement/SceneManager.h"
#include "../Engine/SceneManagement/Transform.h"
#include "../Engine/SceneManagement/WorldObject.h"

Test3D::Test3D(const ApplicationDesc& desc):
  Application(desc)
{
}

void Test3D::OnStart()
{
  _camera.reset(new FpsCamera());
  _camera->SetPerspective(Degree(67.67f), GetWidth(), GetHeight(), 0.1f, 1000.0f);
  _camera->LookAt(Vector3(0.0f, 0.0f, 4.0f), Vector3(0.0f, 0.0f, 0.0f));
  _sceneManager->SetCamera(_camera);
  
  auto model = _sceneManager->CreateObject("cube");
  auto cubeMesh = MeshFactory::CreateCube();
	auto cubeTexture = TextureLoader::LoadFromFile2D("./../../Resources/Textures/crate0_diffuse.png");
	cubeMesh->GetMaterial()->SetTexture("DiffuseMap", cubeTexture);
  auto modelRenderable = std::make_shared<Renderable>();
  modelRenderable->PushMesh(cubeMesh);
  model->AttachRenderable(modelRenderable);

  _inputHandler->BindButtonToState("MoveForward", Button::Key_W);
  _inputHandler->BindButtonToState("MoveBackward", Button::Key_S);
  _inputHandler->BindButtonToState("StrafeLeft", Button::Key_A);
  _inputHandler->BindButtonToState("StrafeRight", Button::Key_D);
  _inputHandler->BindButtonToState("ActivateCameraLook", Button::Button_RMouse);
  _inputHandler->BindAxisToState("CameraLook", Axis::MouseXY);

  _eventDispatcher->Register("MoveForward", [&](const InputEvent& inputEvent, int32 dt)
  {
    _camera->MoveForward(dt);
  });
  _eventDispatcher->Register("MoveBackward", [&](const InputEvent& inputEvent, int32 dt)
  {
    _camera->MoveBackward(dt);
  });
  _eventDispatcher->Register("StrafeLeft", [&](const InputEvent& inputEvent, int32 dt)
  {
    _camera->StrafeLeft(dt);
  });
  _eventDispatcher->Register("StrafeRight", [&](const InputEvent& inputEvent, int32 dt)
  {
    _camera->StrafeRight(dt);
  });
  _eventDispatcher->Register("CameraLook", [&](const InputEvent& inputEvent, int32 dt)
  {
    if (_inputHandler->IsButtonStateActive("ActivateCameraLook"))
    {
      Radian yaw(static_cast<float32>(-inputEvent.AxisPosDelta[1]));
      Radian pitch(static_cast<float32>(-inputEvent.AxisPosDelta[0]));
      _camera->Rotate(yaw, pitch, dt);
    }
  });
}

void Test3D::OnUpdate(uint32 dtMs)
{
  std::stringstream ss;
  ss.precision(4);
  ss << GetAverageFps(dtMs) << " FPS " << GetAverageTickMs(dtMs) << " ms";
}

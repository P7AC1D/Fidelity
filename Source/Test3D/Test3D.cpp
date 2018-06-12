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
#include "../Engine/Rendering/Material.h"
#include "../Engine/Rendering/Renderable.hpp"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/StaticMesh.h"
#include "../Engine/Rendering/Texture.hpp"
#include "../Engine/Utility/AssetManager.h"
#include "../Engine/Utility/ObjLoader.hpp"
#include "../Engine/SceneManagement/Light.h"
#include "../Engine/SceneManagement/FpsCamera.hpp"
#include "../Engine/SceneManagement/OrbitalCamera.hpp"
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
  _camera.reset(new FpsCamera());
  _camera->SetPerspective(Degree(67.67f), GetWidth(), GetHeight(), 0.1f, 1000.0f);
  _camera->LookAt(Vector3(69.0f, 88.0f, 0.8f), Vector3(-28056.1523f, -16285.3330f, -2427.24756f));
  _sceneManager->SetCamera(_camera);
  
  auto model = _sceneManager->CreateObject("cube");
  auto cubeMesh = MeshFactory::CreateCube();
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

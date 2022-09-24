#include "Test3D.h"

#include <iomanip>
#include <sstream>

static const float32 CAMERA_ROTATION_FACTOR = 0.01f;
static const float32 CAMERA_ZOOM_FACTOR = 0.01f;
static const float32 CAMERA_LOOK_SENSITIVITY = 0.005f;
static const float32 CAMERA_MOVE_FACTOR = 0.10f;
static const float32 CAMERA_MOVE_SPRINT_FACTOR = 1.0f;

Test3D::Test3D(const ApplicationDesc &desc) : Application(desc),
                                              _cameraTarget(Vector3::Zero)
{
}

void Test3D::OnStart()
{
  GameObject &root = _scene.getRoot();

  _camera = &GameObjectBuilder(_scene)
                 .withName("mainCamera")
                 .withComponent(_scene.createComponent<Camera>()
                                    .setPerspective(Degree(67.67f), GetWidth(), GetHeight(), 0.1f, 1000.0f))
                 .withPosition(Vector3(-15.0f, 17.0f, -11.0f))
                 .withTarget(Vector3::Zero)
                 .withRotation(Quaternion(Degree(-123.0f), Degree(36.0f), Degree(138.0f)))
                 .build();
  _scene.addChildToNode(root, *_camera);

  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("directionalLight")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setLightType(LightType::Directional)
                                                     .setColour(Colour(244, 233, 155))
                                                     .setIntensity(0.5f))
                                  .withRotation(Quaternion(Degree(-23), Degree(-73.f), Degree(26.0f)))
                                  .build());
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("light1")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setColour(Colour(150, 25, 25))
                                                     .setRadius(300.0f))
                                  .withPosition(Vector3(0.0f, 100.0f, 0.0f))
                                  .build());
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("light2")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setColour(Colour(25, 150, 25))
                                                     .setRadius(300.0f))
                                  .withPosition(Vector3(-150.0f, 100.0f, -100.0f))
                                  .build());
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("light3")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setColour(Colour(25, 25, 100))
                                                     .setRadius(300.0f))
                                  .withPosition(Vector3(-150.0f, 100.0f, 100.0f))
                                  .build());
  //_scene.addChildToNode(root, ModelLoader::FromFile(_scene, "./Models/Sponza/sponza.obj", true));

  std::shared_ptr<Material> material(new Material());
  material->setDiffuseTexture(LoadTextureFromFile("./Textures/crate0_diffuse.png", true, true));
  material->setNormalTexture(LoadTextureFromFile("./Textures/crate0_normal.png", false, false));
  material->setSpecularTexture(LoadTextureFromFile("./Textures/crate0_bump.png", false, false));

  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("cube1")
                                  .withComponent(_scene.createComponent<Drawable>()
                                                     .setMesh(MeshFactory::CreateCube())
                                                     .setMaterial(material))
                                  .withPosition(Vector3(0, 1, 0))
                                  .build());

  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("cube2")
                                  .withComponent(_scene.createComponent<Drawable>()
                                                     .setMesh(MeshFactory::CreateCube())
                                                     .setMaterial(material))
                                  .withPosition(Vector3(-6, 3.5, 7))
                                  .build());

  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("cube3")
                                  .withComponent(_scene.createComponent<Drawable>()
                                                     .setMesh(MeshFactory::CreateCube())
                                                     .setMaterial(material))
                                  .withPosition(Vector3(6, 1.3, 5.4))
                                  .build());

  std::shared_ptr<Material> floorMaterial(new Material());
  floorMaterial->setDiffuseTexture(LoadTextureFromFile("./Textures/brick_floor_tileable_Base_Color.jpg", true, true));
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("floor")
                                  .withComponent(_scene.createComponent<Drawable>()
                                                     .setMesh(MeshFactory::CreatePlane(100))
                                                     .setMaterial(floorMaterial))
                                  .withPosition(Vector3(50, -5, 50))
                                  .withScale(Vector3(100, 100, 100))
                                  .build());
}

void Test3D::OnUpdate(uint32 dtMs)
{
  Vector2I scrollDelta(_inputHandler->getAxisState(Axis::MouseScrollXY));
  ZoomCamera(scrollDelta[1], dtMs);

  Vector2I currMousePos(_inputHandler->getAxisState(Axis::MouseXY));
  Vector2I mousePosDelta = _lastMousePos - currMousePos;

  if (_inputHandler->isButtonPressed(Button::Key_W))
  {
    float32 deltaX = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    TranslateCamera(deltaX, 0.0f);
  }
  else if (_inputHandler->isButtonPressed(Button::Key_S))
  {
    float32 deltaX = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    TranslateCamera(-deltaX, 0.0f);
  }

  if (_inputHandler->isButtonPressed(Button::Key_D))
  {
    float32 deltaY = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    TranslateCamera(0.0f, deltaY);
  }
  else if (_inputHandler->isButtonPressed(Button::Key_A))
  {
    float32 deltaY = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    TranslateCamera(0.0f, -deltaY);
  }

  if (_inputHandler->isButtonPressed(Button::Button_LMouse))
  {
    RotateCamera(mousePosDelta[0], mousePosDelta[1], dtMs);
  }
  else if (_inputHandler->isButtonPressed(Button::Button_RMouse))
  {
    FpsCameraLook(mousePosDelta[0], mousePosDelta[1], dtMs);
  }

  _lastMousePos = currMousePos;
}

void Test3D::FpsCameraLook(int32 deltaX, int32 deltaY, uint32 dtMs)
{
  if (deltaX == 0 && deltaY == 0)
  {
    return;
  }

  Transform &cameraTransform = _camera->transform();
  float32 velocity(CAMERA_LOOK_SENSITIVITY * static_cast<float32>(dtMs));
  Quaternion qPitch(Vector3(1.0f, 0.0f, 0.0f), -velocity * deltaY);
  Quaternion qYaw(Vector3(0.0f, 1.0f, 0.0f), -velocity * deltaX);

  cameraTransform.setRotation(qPitch * cameraTransform.getRotation() * qYaw);
  _cameraTarget = cameraTransform.getForward();
}

void Test3D::RotateCamera(int32 deltaX, int32 deltaY, uint32 dtMs)
{
  if (deltaX == 0 && deltaY == 0)
  {
    return;
  }

  Transform &cameraTransform = _camera->transform();
  float32 velocity(CAMERA_ROTATION_FACTOR * static_cast<float32>(dtMs));
  Quaternion pitch(cameraTransform.getRight(), velocity * deltaY);
  Quaternion yaw(cameraTransform.getUp(), velocity * deltaX);
  Quaternion rotation(pitch * yaw);
  Vector3 newPosition(rotation.Rotate(cameraTransform.getPosition()));
  cameraTransform.lookAt(newPosition, cameraTransform.getForward());
}

void Test3D::ZoomCamera(int32 delta, uint32 dtMs)
{
  if (delta == 0)
  {
    return;
  }

  Transform &cameraTransform = _camera->transform();
  Vector3 cameraForward = cameraTransform.getForward();
  Vector3 cameraPostion = cameraTransform.getPosition();

  float32 distanceToTarget = Vector3::Length(cameraPostion - _cameraTarget);
  float32 velocity(CAMERA_ZOOM_FACTOR * distanceToTarget * static_cast<float32>(dtMs));
  Vector3 newPosition(cameraPostion + cameraForward * velocity * -delta);

  Vector3 cameraToOldPos(cameraPostion - _cameraTarget);
  Vector3 cameraToNewPos(newPosition - _cameraTarget);
  if (Vector3::Dot(cameraToNewPos, cameraToOldPos) < 0.0f)
  {
    newPosition = _cameraTarget + cameraForward * 0.1f;
  }
  cameraTransform.lookAt(newPosition, _cameraTarget);
}

void Test3D::TranslateCamera(float32 forward, float32 right)
{
  if (forward == 0.0f && right == 0.0f)
  {
    return;
  }

  Transform &cameraTransform = _camera->transform();
  Vector3 cameraForward = cameraTransform.getForward();
  Vector3 cameraRight = cameraTransform.getRight();
  cameraTransform.translate(-cameraForward * forward + cameraRight * right);
}
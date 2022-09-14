#include "Test3D.h"

#include <iomanip>
#include <sstream>

static const float32 CAMERA_ROTATION_FACTOR = 0.005f;
static const float32 CAMERA_ZOOM_FACTOR = 0.01f;

Test3D::Test3D(const ApplicationDesc &desc) : Application(desc),
                                              _cameraTarget(Vector3::Zero)
{
}

void Test3D::OnStart()
{
  _scene.getCamera()
      .setPerspective(Degree(67.67f), GetWidth(), GetHeight(), 0.1f, 2000.0f)
      .getTransform()
      .lookAt(Vector3(-400.0f, 500.0f, 20.0f), _cameraTarget);

  GameObject &root = _scene.getRoot();
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("directionalLight")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setLightType(LightType::Directional)
                                                     .setColour(Colour(244, 233, 155))
                                                     .setIntensity(0.1f))
                                  .withRotation(Quaternion(Degree(-120.0f), 30.0f, 6.0f))
                                  .build());
  //_scene.addChildToNode(root, GameObjectBuilder(_scene)
  //                                .withName("light1")
  //                                .withComponent(_scene.createComponent<Light>()
  //                                                   .setColour(Colour(150, 25, 25))
  //                                                   .setRadius(300.0f))
  //                                .withPosition(Vector3(0.0f, 100.0f, 0.0f))
  //                                .build());
  //_scene.addChildToNode(root, GameObjectBuilder(_scene)
  //                                .withName("light2")
  //                                .withComponent(_scene.createComponent<Light>()
  //                                                   .setColour(Colour(25, 150, 25))
  //                                                   .setRadius(300.0f))
  //                                .withPosition(Vector3(-150.0f, 100.0f, -100.0f))
  //                                .build());
  //_scene.addChildToNode(root, GameObjectBuilder(_scene)
  //                                .withName("light3")
  //                                .withComponent(_scene.createComponent<Light>()
  //                                                   .setColour(Colour(25, 25, 100))
  //                                                   .setRadius(300.0f))
  //                                .withPosition(Vector3(-150.0f, 100.0f, 100.0f))
  //                                .build());
  //_scene.addChildToNode(root, ModelLoader::FromFile(_scene, "./Models/Sponza/sponza.obj", true));

  std::shared_ptr<Material> material(new Material());
  material->setDiffuseTexture(LoadTextureFromFile("./Textures/crate0_diffuse.png", true, true));
  material->setNormalTexture(LoadTextureFromFile("./Textures/crate0_normal.png", false, false));
  material->setSpecularTexture(LoadTextureFromFile("./Textures/crate0_bump.png", false, false));

  //_scene.addChildToNode(root, GameObjectBuilder(_scene)
  //                                .withName("cube1")
  //                                .withComponent(_scene.createComponent<Drawable>()
  //                                                   .setMesh(MeshFactory::CreateCube())
  //                                                   .setMaterial(material))
  //                                .withPosition(Vector3(0, 1, 0))
  //                                .build());

  //_scene.addChildToNode(root, GameObjectBuilder(_scene)
  //                                .withName("cube2")
  //                                .withComponent(_scene.createComponent<Drawable>()
  //                                                   .setMesh(MeshFactory::CreateCube())
  //                                                   .setMaterial(material))
  //                                .withPosition(Vector3(-6, 3.5, 7))
  //                                .build());

  //_scene.addChildToNode(root, GameObjectBuilder(_scene)
  //                                .withName("cube3")
  //                                .withComponent(_scene.createComponent<Drawable>()
  //                                                   .setMesh(MeshFactory::CreateCube())
  //                                                   .setMaterial(material))
  //                                .withPosition(Vector3(6, 1.3, 5.4))
  //                                .build());

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

  _inputHandler->BindButtonToState("ActivateCameraLook", Button::Button_LMouse);
  _inputHandler->BindAxisToState("CameraZoom", Axis::MouseScrollXY);
  _inputHandler->BindAxisToState("CameraLook", Axis::MouseXY);

  _eventDispatcher->Register("CameraZoom", [&](const InputEvent &inputEvent, int32 dt)
                             { ZoomCamera(static_cast<float32>(inputEvent.AxisPosDelta[1]), dt); });

  _eventDispatcher->Register("CameraLook", [&](const InputEvent &inputEvent, int32 dt)
                             {
    if (_inputHandler->IsButtonStateActive("ActivateCameraLook"))
    {
      Radian yaw(static_cast<float32>(-inputEvent.AxisPosDelta[1]));
      Radian pitch(static_cast<float32>(-inputEvent.AxisPosDelta[0]));
      RotateCamera(yaw, pitch, dt);
    } });
}

void Test3D::OnUpdate(uint32 dtMs)
{
  std::stringstream ss;
  ss.precision(4);
  ss << GetAverageFps(dtMs) << " FPS " << GetAverageTickMs(dtMs) << " ms";
}

void Test3D::RotateCamera(const Degree &deltaX, const Degree &deltaY, int32 dtMs)
{
  Transform &cameraTransform = _scene.getCamera().getTransform();
  float32 velocity(CAMERA_ROTATION_FACTOR * static_cast<float32>(dtMs));
  Quaternion pitch(cameraTransform.getRight(), velocity * deltaX.InRadians());
  Quaternion yaw(cameraTransform.getUp(), velocity * deltaY.InRadians());
  Quaternion rotation(pitch * yaw);
  Vector3 newPosition(rotation.Rotate(cameraTransform.getPosition()));
  cameraTransform.lookAt(newPosition, _cameraTarget);
}

void Test3D::ZoomCamera(float32 delta, int32 dtMs)
{
  Transform &cameraTransform = _scene.getCamera().getTransform();
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
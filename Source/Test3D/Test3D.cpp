#include "Test3D.h"

#include <iomanip>
#include <random>
#include <sstream>

Test3D::Test3D(const ApplicationDesc &desc) : Application(desc)

{
}

void Test3D::onStart()
{
  std::shared_ptr<GameObject> root(_scene.getRootGameObject());
  _camera = GameObjectBuilder()
                .withName("mainCamera")
                .withComponent(CameraBuilder()
                                   .withFar(200.0f)
                                   .withNear(0.1f)
                                   .withWidth(getWidth())
                                   .withHeight(getHeight())
                                   .withFov(Degree(67.67f))
                                   .build())
                .withPosition(Vector3(-15.0f, 17.0f, -11.0f))
                .withTarget(Vector3::Zero)
                .withRotation(Quaternion(Degree(-123.0f), Degree(36.0f), Degree(138.0f)))
                .build();
  root->addChildNode(_camera);
  _scene.setMainCamera(_camera->getComponent<Camera>());

  root->addChildNode(GameObjectBuilder()
                         .withName("directionalLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(244, 233, 155))
                                            .withLightType(LightType::Directional)
                                            .withIntensity(1.0f)
                                            .build())
                         .withRotation(Quaternion(Degree(-23), Degree(-73.f), Degree(26.0f)))
                         .build());
  root->addChildNode(GameObjectBuilder()
                         .withName("redLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(150, 25, 25))
                                            .withRadius(25.0f)
                                            .withLightType(LightType::Point)
                                            .withIntensity(10.0f)
                                            .build())
                         .withPosition(Vector3(0.0f, 1.0f, 0.0f))
                         .build());
  root->addChildNode(GameObjectBuilder()
                         .withName("greenLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(25, 150, 25))
                                            .withRadius(25.0f)
                                            .withLightType(LightType::Point)
                                            .withIntensity(10.0f)
                                            .build())
                         .withPosition(Vector3(-1.5f, 1.0f, -1.0f))
                         .build());
  root->addChildNode(GameObjectBuilder()
                         .withName("blueLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(25, 25, 100))
                                            .withRadius(25.0f)
                                            .withLightType(LightType::Point)
                                            .withIntensity(10.0f)
                                            .build())
                         .withPosition(Vector3(-1.5f, 1.0f, 1.0f))
                         .build());

  std::shared_ptr<Material> material(new Material());
  material->setDiffuseTexture(loadTextureFromFile("./Textures/crate0_diffuse.png", true, true));
  material->setNormalTexture(loadTextureFromFile("./Textures/crate0_normal.png", false, false));
  material->setMetallicTexture(loadTextureFromFile("./Textures/crate0_bump.png", false, false));

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dist(-25, 25);
  for (uint32 i = 0; i < 100; i++)
  {
    root->addChildNode(GameObjectBuilder()
                           .withName("cube" + std::to_string(i))
                           .withComponent(DrawableBuilder()
                                              .withMesh(MeshFactory::createCube())
                                              .withMaterial(material)
                                              .build())
                           .withPosition(Vector3(dist(gen), std::fabsf(dist(gen)), dist(gen)))
                           .withRotation(Quaternion(Degree(dist(gen)), Degree(dist(gen)), Degree(dist(gen))))
                           .build());
  }

  std::shared_ptr<Material> floorMaterial(new Material());
  floorMaterial->setDiffuseTexture(loadTextureFromFile("./Textures/brick_floor_tileable_Base_Color.jpg", true, true));
  root->addChildNode(GameObjectBuilder()
                         .withName("floor")
                         .withComponent(DrawableBuilder()
                                            .withMaterial(floorMaterial)
                                            .withMesh(MeshFactory::createPlane(100))
                                            .build())
                         .withPosition(Vector3(50, -5, 50))
                         .withScale(Vector3(100, 100, 100))
                         .build());
}

void Test3D::onUpdate(uint32 dtMs)
{
  Vector2I mousePosDelta = _lastMousePos - _currentMousePos;

  if (_inputHandler->isButtonPressed(Button::Key_W))
  {
    float32 deltaX = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    translateCamera(deltaX, 0.0f);
  }
  else if (_inputHandler->isButtonPressed(Button::Key_S))
  {
    float32 deltaX = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    translateCamera(-deltaX, 0.0f);
  }

  if (_inputHandler->isButtonPressed(Button::Key_D))
  {
    float32 deltaY = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    translateCamera(0.0f, deltaY);
  }
  else if (_inputHandler->isButtonPressed(Button::Key_A))
  {
    float32 deltaY = static_cast<float32>(dtMs) * (_inputHandler->isButtonPressed(Button::Key_LShift) ? CAMERA_MOVE_SPRINT_FACTOR : CAMERA_MOVE_FACTOR);
    translateCamera(0.0f, -deltaY);
  }

  if (_inputHandler->isButtonPressed(Button::Button_RMouse))
  {
    fpsCameraLook(mousePosDelta[0], mousePosDelta[1], dtMs);
  }
}
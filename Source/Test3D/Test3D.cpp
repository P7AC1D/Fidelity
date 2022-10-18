#include "Test3D.h"

#include <iomanip>
#include <random>
#include <sstream>

Test3D::Test3D(const ApplicationDesc &desc) : Application(desc)

{
}

void Test3D::onStart()
{
  GameObject &root = _scene.getRoot();

  _camera = &GameObjectBuilder(_scene)
                 .withName("mainCamera")
                 .withComponent(_scene.createComponent<Camera>()
                                    .setPerspective(Degree(67.67f), getWidth(), getHeight(), 0.1f, 200.0f))
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
                                                     .setRadius(25.0f))
                                  .withPosition(Vector3(0.0f, 1.0f, 0.0f))
                                  .build());
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("light2")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setColour(Colour(25, 150, 25))
                                                     .setRadius(25.0f))
                                  .withPosition(Vector3(-1.5f, 1.0f, -1.0f))
                                  .build());
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("light3")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setColour(Colour(25, 25, 100))
                                                     .setRadius(25.0f))
                                  .withPosition(Vector3(-1.5f, 1.0f, 1.0f))
                                  .build());

  std::shared_ptr<Material> material(new Material());
  material->setDiffuseTexture(loadTextureFromFile("./Textures/crate0_diffuse.png", true, true));
  material->setNormalTexture(loadTextureFromFile("./Textures/crate0_normal.png", false, false));
  material->setSpecularTexture(loadTextureFromFile("./Textures/crate0_bump.png", false, false));

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dist(-25, 25);
  for (uint32 i = 0; i < 100; i++)
  {
    _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                    .withName("cube" + std::to_string(i))
                                    .withComponent(_scene.createComponent<Drawable>()
                                                       .setMesh(MeshFactory::createCube())
                                                       .setMaterial(material))
                                    .withPosition(Vector3(dist(gen), std::fabsf(dist(gen)), dist(gen)))
                                    .withRotation(Quaternion(Degree(dist(gen)), Degree(dist(gen)), Degree(dist(gen))))
                                    .build());
  }

  std::shared_ptr<Material> floorMaterial(new Material());
  floorMaterial->setDiffuseTexture(loadTextureFromFile("./Textures/brick_floor_tileable_Base_Color.jpg", true, true));
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("floor")
                                  .withComponent(_scene.createComponent<Drawable>()
                                                     .setMesh(MeshFactory::createPlane(100))
                                                     .setMaterial(floorMaterial))
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
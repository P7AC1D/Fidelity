#include "Sponza.h"

#include <iomanip>
#include <random>
#include <sstream>

Sponza::Sponza(const ApplicationDesc &desc) : Application(desc)
{
}

void Sponza::onStart()
{
  GameObject &root = _scene.getRoot();

  _camera = &GameObjectBuilder(_scene)
                 .withName("mainCamera")
                 .withComponent(_scene.createComponent<Camera>()
                                    .setPerspective(Degree(67.67f), getWidth(), getHeight(), 0.1f, 500.0f))
                 .withPosition(Vector3(-105.0f, 70.0f, 9.0f))
                 .withTarget(Vector3::Zero)
                 .withRotation(Quaternion(Degree(59.552), Degree(53.438), Degree(53.802)))
                 .build();
  _scene.addChildToNode(root, *_camera);

  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("directionalLight")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setLightType(LightType::Directional)
                                                     .setColour(Colour(244, 233, 155))
                                                     .setIntensity(0.5f))
                                  .withRotation(Quaternion(Degree(36.139), Degree(-72.174), Degree(-30.861f)))
                                  .build());
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("light1")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setColour(Colour(150, 25, 25))
                                                     .setRadius(70.0f))
                                  .withPosition(Vector3(95.0f, 8.0f, 0.0f))
                                  .build());
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("light2")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setColour(Colour(25, 150, 25))
                                                     .setRadius(70.0f))
                                  .withPosition(Vector3(-51.0f, 8.0f, 0.0f))
                                  .build());
  _scene.addChildToNode(root, GameObjectBuilder(_scene)
                                  .withName("light3")
                                  .withComponent(_scene.createComponent<Light>()
                                                     .setColour(Colour(25, 25, 100))
                                                     .setRadius(70.0f))
                                  .withPosition(Vector3(12.0f, 8.0f, 0.0f))
                                  .build());

  auto &sponzaNode = ModelLoader::fromFile(_scene, "./Models/Sponza/sponza.obj", true);
  sponzaNode.transform().setScale(Vector3(0.1, 0.1, 0.1));
  _scene.addChildToNode(root, sponzaNode);
}

void Sponza::onUpdate(uint32 dtMs)
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
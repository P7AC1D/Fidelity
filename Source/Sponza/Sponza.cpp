#include "Sponza.h"

#include <iomanip>
#include <random>
#include <sstream>

Sponza::Sponza(const ApplicationDesc &desc) : Application(desc)
{
}

void Sponza::onStart()
{
  std::shared_ptr<GameObject> root(_scene.getRootGameObject());
  _camera = GameObjectBuilder()
                .withName("mainCamera")
                .withComponent(CameraBuilder()
                                   .withFar(500.0f)
                                   .withNear(0.1f)
                                   .withWidth(getWidth())
                                   .withHeight(getHeight())
                                   .withFov(Degree(67.67f))
                                   .build())
                .withPosition(Vector3(-105.0f, 70.0f, 9.0f))
                .withTarget(Vector3::Zero)
                .withRotation(Quaternion(Degree(59.552), Degree(53.438), Degree(53.802)))
                .build();
  root->addChildNode(_camera);

  root->addChildNode(GameObjectBuilder()
                         .withName("directionalLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(244, 233, 155))
                                            .withLightType(LightType::Directional)
                                            .withIntensity(1.0f)
                                            .build())
                         .withRotation(Quaternion(Degree(36.139), Degree(-72.174), Degree(-30.861f)))
                         .build());

  root->addChildNode(GameObjectBuilder()
                         .withName("redLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(150, 25, 25))
                                            .withRadius(25.0f)
                                            .withLightType(LightType::Point)
                                            .withIntensity(10.0f)
                                            .build())
                         .withPosition(Vector3(95.0f, 8.0f, 0.0f))
                         .build());
  root->addChildNode(GameObjectBuilder()
                         .withName("greenLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(25, 150, 25))
                                            .withRadius(25.0f)
                                            .withLightType(LightType::Point)
                                            .withIntensity(10.0f)
                                            .build())
                         .withPosition(Vector3(-51.0f, 8.0f, 0.0f))
                         .build());
  root->addChildNode(GameObjectBuilder()
                         .withName("blueLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(25, 25, 100))
                                            .withRadius(25.0f)
                                            .withLightType(LightType::Point)
                                            .withIntensity(10.0f)
                                            .build())
                         .withPosition(Vector3(12.0f, 8.0f, 0.0f))
                         .build());

  std::shared_ptr<GameObject> sponzaNode = ModelLoader::fromFile(_scene, "./Models/sponza_pbr/sponza.obj", true);
  sponzaNode->transform().setScale(Vector3(0.1, 0.1, 0.1));
  root->addChildNode(sponzaNode);
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
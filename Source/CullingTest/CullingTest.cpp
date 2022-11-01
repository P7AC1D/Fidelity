#include "CullingTest.h"

#include <iomanip>
#include <random>
#include <sstream>

CullingTest::CullingTest(const ApplicationDesc &desc) : Application(desc)
{
}

void CullingTest::onStart()
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

  root->addChildNode(GameObjectBuilder()
                         .withName("directionalLight")
                         .withComponent(LightBuilder()
                                            .withColour(Colour(244, 233, 155))
                                            .withLightType(LightType::Directional)
                                            .withIntensity(1.0f)
                                            .build())
                         .withRotation(Quaternion(Degree(36.139), Degree(-72.174), Degree(-30.861f)))
                         .build());

  std::shared_ptr<Material> material(new Material());
  material->setDiffuseTexture(loadTextureFromFile("./Textures/crate0_diffuse.png", true, true));
  material->setNormalTexture(loadTextureFromFile("./Textures/crate0_normal.png", false, false));
  material->setMetallicTexture(loadTextureFromFile("./Textures/crate0_bump.png", false, false));

  uint32 count = 0;
  for (int32 i = -5; i < 5; i++)
  {
    for (int32 j = -5; j < 5; j++)
    {
      for (int32 k = -5; k < 5; k++)
      {
        root->addChildNode(GameObjectBuilder()
                               .withName("cube" + std::to_string(count++))
                               .withComponent(DrawableBuilder()
                                                  .withMesh(MeshFactory::createCube())
                                                  .withMaterial(material)
                                                  .build())
                               .withPosition(Vector3(3 * i, 3 * j, 3 * k))
                               .build());
      }
    }
  }
}

void CullingTest::onUpdate(uint32 dtMs)
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
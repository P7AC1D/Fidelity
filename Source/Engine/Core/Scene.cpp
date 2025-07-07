#include <algorithm>
#include <chrono>

#include "../Rendering/CameraComponent.h"
#include "../Rendering/DrawableComponent.h"
#include "../Rendering/LightComponent.h"
#include "../Rendering/Renderer.h"
#include "../UI/ImGui/imgui.h"
#include "../Maths/Ray.hpp"
#include "../Maths/AABB.hpp"
#include "Scene.h"
#include "TransformComponent.h"
#include "InputHandler.h"

/// @brief Builds a projected ray in world space from the a set of mouse coordinates in screen space.
/// @param mouseCoords The current mouse coordinates in screen space.
/// @param windowDims The current window's dimensions.
/// @param camera The current active camera.
/// @return The projected ray in world space.
Ray buildRayFromMouseCoords(const Vector2I &mouseCoords, const Vector2I &windowDims, const CameraComponent &camera)
{
  float32 x = (static_cast<float32>(mouseCoords.X) / windowDims.X) * 2.0f - 1.0f;
  float32 y = 1.0f - (static_cast<float32>(mouseCoords.Y) / windowDims.Y) * 2.0f;

  // We want our ray's z to point forwards - this is usually the negative z direction in OpenGL style.
  Vector4 rayClip(x, y, -1.0f, 1.0f);

  Vector4 rayView = camera.getProj().Inverse() * rayClip;
  rayView.Z = -1.0f;
  rayView.W = 0.0f;

  Vector3 rayWorld = Vector3(camera.getView().Inverse() * rayView);
  rayWorld.Normalize();

  return Ray(camera.getWorldPosition(), rayWorld);
}

Scene::Scene(const std::shared_ptr<InputHandler> &inputHandler)
    : _inputHandler(inputHandler)
{
  _componentManager = std::make_unique<ComponentManager>();
}

Scene::~Scene() = default;

bool Scene::init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice)
{
  _windowDims = windowDims;
  _renderDevice = renderDevice;

  // Create renderer
  _renderer = std::make_shared<Renderer>(windowDims);
  if (!_renderer->init(renderDevice))
  {
    return false;
  }

  // Create root GameObject
  _rootObject = std::make_unique<GameObject>("Root", 0, _componentManager.get());
  _rootObject->addComponent<TransformComponent>();

  return true;
}

GameObject &Scene::createGameObject(const std::string &name)
{
  auto gameObject = std::make_unique<GameObject>(name, _nextGameObjectId++, _componentManager.get());

  // Add basic transform component
  gameObject->addComponent<TransformComponent>();

  GameObject &ref = *gameObject;
  _gameObjects.push_back(std::move(gameObject));
  _objectAddedToScene = true;

  return ref;
}

void Scene::addChild(GameObject &parent, std::unique_ptr<GameObject> child)
{
  parent.addChild(std::move(child));
}

void Scene::update(float32 dt)
{
  auto startTime = std::chrono::high_resolution_clock::now();

  // Update root object (which will update all children)
  if (_rootObject)
  {
    _rootObject->update(dt);
  }

  // Update all standalone GameObjects
  for (auto &gameObject : _gameObjects)
  {
    gameObject->update(dt);
  }

  auto endTime = std::chrono::high_resolution_clock::now();
  _scenePrepDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
}

void Scene::drawFrame()
{
  auto startTime = std::chrono::high_resolution_clock::now();

  if (!_renderer)
    return;

  // Get main camera
  auto *mainCamera = getMainCamera();
  if (!mainCamera)
    return;

  // Perform object picking if mouse coordinates are set
  performObjectPicker(*mainCamera);

  // Get raw pointers from component system
  auto rawLights = getLights();
  auto rawDrawables = getDrawables();

  // Create temporary shared_ptrs with no-op deleters for renderer compatibility
  // This is a bridge solution while we migrate the renderer to raw pointers
  std::vector<std::shared_ptr<DrawableComponent>> sharedDrawables;
  std::vector<std::shared_ptr<LightComponent>> sharedLights;

  for (auto *drawable : rawDrawables)
  {
    sharedDrawables.emplace_back(drawable, [](DrawableComponent *) {});
  }

  for (auto *light : rawLights)
  {
    sharedLights.emplace_back(light, [](LightComponent *) {});
  }

  auto sharedCamera = std::shared_ptr<CameraComponent>(mainCamera, [](CameraComponent *) {});

  auto endTime = std::chrono::high_resolution_clock::now();
  _scenePrepDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

  // Call renderer with shared_ptrs (temporary bridge solution)
  _renderer->drawFrame(_renderDevice, sharedDrawables, sharedLights, sharedCamera);
}

void Scene::drawDebugUi()
{
  ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvail().x, 300), false, ImGuiWindowFlags_HorizontalScrollbar);
  if (_rootObject)
  {
    drawSceneGraphUi(*_rootObject);
  }
  drawGameObjectInspector(_selectedGameObject);
  ImGui::EndChild();

  if (_renderer)
  {
    _renderer->drawDebugUi();
  }

  ImGui::Separator();
  {
    if (ImGui::CollapsingHeader("Frame Profiler"))
    {
      float32 scenePrepDuration = static_cast<float32>(_scenePrepDuration) * 1e-6f;
      ImGui::Text("Scene Prep: (%.3f ms)", scenePrepDuration);
      float32 totalDuration = scenePrepDuration;
      for (const auto &timings : _renderer->getRenderPassTimings())
      {
        float32 duration = static_cast<float32>(timings.Duration) * 1e-6f;
        ImGui::Text("%s: (%.3f ms)", timings.Name.c_str(), duration);
        totalDuration += duration;
      }
      ImGui::Text("All: (%.3f ms)", totalDuration);
    }
  }
}

std::vector<CameraComponent *> Scene::getCameras()
{
  return collectComponents<CameraComponent>();
}

std::vector<LightComponent *> Scene::getLights()
{
  return collectComponents<LightComponent>();
}

std::vector<DrawableComponent *> Scene::getDrawables()
{
  return collectComponents<DrawableComponent>();
}

CameraComponent *Scene::getMainCamera()
{
  auto cameras = getCameras();
  return cameras.empty() ? nullptr : cameras[0];
}

void Scene::performObjectPicker(const CameraComponent &camera)
{
  // Only perform picking if mouse coordinates are valid and input handler is available
  if (_mouseCoordinates.X < 0 || _mouseCoordinates.Y < 0 || !_inputHandler)
    return;

  // Build ray from mouse coordinates
  Ray ray = buildRayFromMouseCoords(_mouseCoordinates, _windowDims, camera);

  std::vector<std::pair<float32, GameObject *>> rayCastedObjects;
  Vector3 cameraPos = camera.getWorldPosition();

  // Check all GameObjects (including root and its children)
  checkGameObjectForPicking(*_rootObject, ray, cameraPos, rayCastedObjects);

  for (auto &gameObject : _gameObjects)
  {
    checkGameObjectForPicking(*gameObject, ray, cameraPos, rayCastedObjects);
  }

  // Only process selection if left mouse button is pressed
  if (!rayCastedObjects.empty() && _inputHandler->isButtonPressed(Button::Button_LMouse))
  {
    // Sort by distance (closest first)
    std::sort(rayCastedObjects.begin(), rayCastedObjects.end(),
              [](const std::pair<float32, GameObject *> &a, const std::pair<float32, GameObject *> &b)
              {
                return a.first < b.first;
              });

    // Clear previous selection
    setAabbDrawOnGameObject(_selectedGameObject, false);

    // Select the closest object
    _selectedGameObject = rayCastedObjects.front().second;

    // Enable AABB drawing for selected object
    setAabbDrawOnGameObject(_selectedGameObject, true);
  }
}

void Scene::checkGameObjectForPicking(GameObject &gameObject, const Ray &ray, const Vector3 &cameraPos, std::vector<std::pair<float32, GameObject *>> &results)
{
  // Check if this GameObject has a DrawableComponent
  if (auto *drawable = gameObject.tryGetComponent<DrawableComponent>())
  {
    // Get the world AABB for this drawable
    auto worldBounds = drawable->getWorldBounds();

    // Check ray intersection with AABB
    if (ray.Intersects(worldBounds))
    {
      // Calculate distance from camera for sorting
      Vector3 objectPos = gameObject.transform().getPosition();
      float32 distance = (objectPos - cameraPos).Length();

      results.emplace_back(distance, &gameObject);
    }
  }

  // Recursively check children
  for (const auto &child : gameObject.getChildren())
  {
    checkGameObjectForPicking(*child, ray, cameraPos, results);
  }
}

void Scene::setAabbDrawOnGameObject(GameObject *gameObject, bool enableAabbDraw)
{
  if (!gameObject)
    return;

  // Try to get DrawableComponent and enable/disable AABB drawing
  if (auto *drawable = gameObject->tryGetComponent<DrawableComponent>())
  {
    drawable->enableDrawAabb(enableAabbDraw);
  }
}

void Scene::drawSceneGraphUi(GameObject &gameObject, int depth)
{
  // Create indentation for hierarchy
  std::string indent(depth * 2, ' ');
  std::string label = indent + gameObject.getName();

  // Create tree node
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
  if (gameObject.getChildren().empty())
  {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }
  if (&gameObject == _selectedGameObject)
  {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);

  // Handle selection
  if (ImGui::IsItemClicked())
  {
    _selectedGameObject = &gameObject;
  }

  // Draw children if node is open
  if (nodeOpen)
  {
    for (const auto &child : gameObject.getChildren())
    {
      drawSceneGraphUi(*child, depth + 1);
    }
    ImGui::TreePop();
  }
}

void Scene::drawGameObjectInspector(GameObject *selectedGameObject)
{
  if (!selectedGameObject)
    return;

  // GameObject properties
  ImGui::Text("Name: %s", selectedGameObject->getName().c_str());
  ImGui::Text("ID: %llu", selectedGameObject->getIndex());

  bool active = selectedGameObject->isActive();
  if (ImGui::Checkbox("Active", &active))
  {
    selectedGameObject->setActive(active);
  }

  ImGui::Spacing();

  // Draw all components
  selectedGameObject->drawInspector();
}

std::vector<Scene::DrawableDistance> Scene::sortDrawablesByDistance(const CameraComponent &camera)
{
  std::vector<DrawableDistance> drawables;
  auto allDrawables = getDrawables();

  Vector3 cameraPos = camera.getWorldPosition();

  for (auto *drawable : allDrawables)
  {
    if (auto transform = drawable->getTransformComponent().lock())
    {
      Vector3 objectPos = transform->getPosition();
      float32 distance = (objectPos - cameraPos).Length();
      drawables.emplace_back(distance, drawable);
    }
  }

  // Sort by distance (back to front for transparency)
  std::sort(drawables.begin(), drawables.end(),
            [](const DrawableDistance &a, const DrawableDistance &b)
            {
              return a.distance > b.distance;
            });

  return drawables;
}

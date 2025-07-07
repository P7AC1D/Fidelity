#include <algorithm>
#include <chrono>

#include "../Rendering/CameraComponent.h"
#include "../Rendering/DrawableComponent.h"
#include "../Rendering/LightComponent.h"
#include "../Rendering/Renderer.h"
#include "../UI/ImGui/imgui.h"
#include "Scene.h"
#include "TransformComponent.h"

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
  if (!_renderer)
    return;

  // Get main camera
  auto *mainCamera = getMainCamera();
  if (!mainCamera)
    return;

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

  // Call renderer with shared_ptrs (temporary bridge solution)
  _renderer->drawFrame(_renderDevice, sharedDrawables, sharedLights, sharedCamera);
}

void Scene::drawDebugUi()
{
  if (ImGui::Begin("Scene"))
  {
    // Scene statistics
    ImGui::Text("Scene Statistics");
    ImGui::Separator();
    ImGui::Text("GameObjects: %zu", _gameObjects.size() + 1); // +1 for root
    ImGui::Text("Cameras: %zu", getCameras().size());
    ImGui::Text("Lights: %zu", getLights().size());
    ImGui::Text("Drawables: %zu", getDrawables().size());
    ImGui::Text("Scene Prep Time: %llu μs", _scenePrepDuration);

    ImGui::Spacing();

    // Scene graph
    ImGui::Text("Scene Graph");
    ImGui::Separator();
    if (_rootObject)
    {
      drawSceneGraphUi(*_rootObject);
    }

    ImGui::Spacing();

    // Selected object inspector
    if (_selectedGameObject)
    {
      ImGui::Text("Inspector");
      ImGui::Separator();
      drawGameObjectInspector(_selectedGameObject);
    }
  }
  ImGui::End();
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
  // TODO: Implement object picking using the camera's frustum
  // This would involve ray casting from mouse coordinates
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

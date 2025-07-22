#include <algorithm>
#include <chrono>

#include "ComponentBase.inl"
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
    : _inputHandler(inputHandler), _mouseCoordinates(-1, -1)
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

  return true;
}

GameObject &Scene::createGameObject(const std::string &name)
{
  auto gameObject = std::make_unique<GameObject>(name, _nextGameObjectId++, _componentManager.get());

  GameObject &ref = *gameObject;
  _gameObjects.push_back(std::move(gameObject));
  _objectAddedToScene = true;

  // Apply current AABB drawing state to new object if enabled
  if (_drawAllAABBs)
  {
    setAabbDrawOnGameObject(&ref, true);
  }

  // Invalidate component caches since scene structure changed
  invalidateComponentCaches();

  return ref;
}

void Scene::addChild(GameObject &parent, std::unique_ptr<GameObject> child)
{
  // Apply current AABB drawing state to child object if enabled
  if (_drawAllAABBs)
  {
    setAabbDrawOnGameObjectRecursive(child.get(), true);
  }
  
  parent.addChild(std::move(child));
  // Invalidate component caches since scene structure changed
  invalidateComponentCaches();
}

GameObject &Scene::createChildGameObject(GameObject &parent, const std::string &name)
{
  auto child = std::make_unique<GameObject>(name, _nextGameObjectId++, _componentManager.get());
  GameObject &ref = *child;
  
  // Apply current AABB drawing state to child object if enabled
  if (_drawAllAABBs)
  {
    setAabbDrawOnGameObject(&ref, true);
  }
  
  parent.addChild(std::move(child));

  // Invalidate component caches since scene structure changed
  invalidateComponentCaches();

  return ref;
}

void Scene::update(float32 dt)
{
  auto startTime = std::chrono::high_resolution_clock::now();

  // Update all GameObjects
  for (auto &gameObject : _gameObjects)
  {
    gameObject->update(dt);
  }

  auto endTime = std::chrono::high_resolution_clock::now();
  _scenePrepDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
}

void Scene::updateObjectPicking()
{
  // Get main camera
  auto *mainCamera = getMainCamera();
  if (!mainCamera)
    return;

  // Perform object picking if mouse coordinates are set
  performObjectPicker(*mainCamera);
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

  // Get component lists (now cached)
  const auto &rawLights = getLights();
  const auto &rawDrawables = getDrawables();

  // Use static caches to avoid per-frame allocations and shared_ptr construction
  static std::vector<std::shared_ptr<DrawableComponent>> sharedDrawables;
  static std::vector<std::shared_ptr<LightComponent>> sharedLights;

  // Create a static no-op deleter to avoid lambda allocation on every reset
  static auto noOpDeleter = [](auto *) {}; // This lambda is created once and reused

  // Resize vectors if needed (this only allocates when size changes)
  if (sharedDrawables.size() != rawDrawables.size())
  {
    sharedDrawables.resize(rawDrawables.size());
  }

  if (sharedLights.size() != rawLights.size())
  {
    sharedLights.resize(rawLights.size());
  }

  // Reuse existing shared_ptr objects, just update the raw pointer
  for (size_t i = 0; i < rawDrawables.size(); ++i)
  {
    sharedDrawables[i].reset(rawDrawables[i], noOpDeleter);
  }

  for (size_t i = 0; i < rawLights.size(); ++i)
  {
    sharedLights[i].reset(rawLights[i], noOpDeleter);
  }

  auto sharedCamera = std::shared_ptr<CameraComponent>(mainCamera, noOpDeleter);

  auto endTime = std::chrono::high_resolution_clock::now();
  _scenePrepDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

  // Call renderer with shared_ptrs (temporary bridge solution)
  _renderer->drawFrame(_renderDevice, sharedDrawables, sharedLights, sharedCamera);
}

void Scene::drawDebugUi()
{
  ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvail().x, 300), false, ImGuiWindowFlags_HorizontalScrollbar);

  // Draw only root game objects (those without parents)
  // Child objects will be drawn recursively by drawSceneGraphUi
  for (const auto &gameObject : _gameObjects)
  {
    // Only draw if this GameObject has no parent (i.e., it's a root object)
    if (gameObject->getParent() == nullptr)
    {
      drawSceneGraphUi(*gameObject);
    }
  }

  ImGui::EndChild();

  // Draw the inspector in a separate window
  drawGameObjectInspector(_selectedGameObject);

  if (_renderer)
  {
    _renderer->drawDebugUi();
  }

  ImGui::Separator();
  
  // Scene-wide controls
  if (ImGui::CollapsingHeader("Scene Debug Controls"))
  {
    bool currentState = getDrawAllAABBs();
    if (ImGui::Checkbox("Draw All AABBs", &currentState))
    {
      setDrawAllAABBs(currentState);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
      ImGui::SetTooltip("Temporarily enable AABB drawing for all objects in the scene");
    }
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
  rebuildComponentCaches();
  return _cachedCameras;
}

std::vector<LightComponent *> Scene::getLights()
{
  rebuildComponentCaches();
  return _cachedLights;
}

std::vector<DrawableComponent *> Scene::getDrawables()
{
  rebuildComponentCaches();
  return _cachedDrawables;
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

  // Check only root GameObjects (children will be checked recursively)
  for (auto &gameObject : _gameObjects)
  {
    if (gameObject->getParent() == nullptr)
    {
      checkGameObjectForPicking(*gameObject, ray, cameraPos, rayCastedObjects);
    }
  }

  // Only process selection if left mouse button was just clicked (not held)
  if (!rayCastedObjects.empty() && _inputHandler->wasButtonJustPressed(Button::Button_LMouse))
  {
    // Sort by distance (closest first)
    std::sort(rayCastedObjects.begin(), rayCastedObjects.end(),
              [](const std::pair<float32, GameObject *> &a, const std::pair<float32, GameObject *> &b)
              {
                return a.first < b.first;
              });

    // Check if we're clicking at the same mouse position as last time
    bool samePosition = (_lastPickMousePos.X == _mouseCoordinates.X && _lastPickMousePos.Y == _mouseCoordinates.Y);

    // Extract just the GameObject pointers for comparison
    std::vector<GameObject *> currentObjects;
    for (const auto &pair : rayCastedObjects)
    {
      currentObjects.push_back(pair.second);
    }

    // If same position and same objects, cycle to next object
    if (samePosition && currentObjects == _lastPickedObjects && !_lastPickedObjects.empty())
    {
      _currentPickIndex = (_currentPickIndex + 1) % static_cast<int32>(_lastPickedObjects.size());
    }
    else
    {
      // New position or different objects, start from the beginning
      _lastPickedObjects = currentObjects;
      _lastPickMousePos = _mouseCoordinates;
      _currentPickIndex = 0;
    }

    // Clear previous selection
    setAabbDrawOnGameObject(_selectedGameObject, false);

    // Select the object at current index
    _selectedGameObject = _lastPickedObjects[_currentPickIndex];

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

void Scene::setAabbDrawOnAllObjects(bool enableAabbDraw)
{
  // Apply to all root GameObjects and their children recursively
  for (auto &gameObject : _gameObjects)
  {
    setAabbDrawOnGameObjectRecursive(gameObject.get(), enableAabbDraw);
  }
}

void Scene::setAabbDrawOnGameObjectRecursive(GameObject *gameObject, bool enableAabbDraw)
{
  if (!gameObject)
    return;

  // Enable/disable AABB drawing on this GameObject
  setAabbDrawOnGameObject(gameObject, enableAabbDraw);

  // Recursively apply to all children
  for (const auto &child : gameObject->getChildren())
  {
    setAabbDrawOnGameObjectRecursive(child.get(), enableAabbDraw);
  }
}

void Scene::setDrawAllAABBs(bool enable)
{
  if (_drawAllAABBs != enable)
  {
    _drawAllAABBs = enable;
    setAabbDrawOnAllObjects(enable);
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
    // Clear previous selection
    setAabbDrawOnGameObject(_selectedGameObject, false);

    // Select new object
    _selectedGameObject = &gameObject;

    // Enable AABB drawing for selected object
    setAabbDrawOnGameObject(_selectedGameObject, true);
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

  ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

  // Position the inspector window on the right side of the screen
  ImVec2 screenSize = ImGui::GetIO().DisplaySize;
  auto windowPos = ImVec2(screenSize.x - ImGui::GetWindowWidth(), 0);
  ImGui::SetWindowPos(windowPos);

  // GameObject properties
  ImGui::Separator();
  ImGui::Text("Name: %s", selectedGameObject->getName().c_str());
  ImGui::Text("ID: %llu", selectedGameObject->getIndex());
  ImGui::Separator();

  bool active = selectedGameObject->isActive();
  if (ImGui::Checkbox("Active", &active))
  {
    selectedGameObject->setActive(active);
  }

  ImGui::Spacing();

  // Draw all components
  selectedGameObject->drawInspector();

  ImGui::End();
}

std::vector<Scene::DrawableDistance> Scene::sortDrawablesByDistance(const CameraComponent &camera)
{
  std::vector<DrawableDistance> drawables;
  auto allDrawables = getDrawables();

  Vector3 cameraPos = camera.getWorldPosition();

  for (auto *drawable : allDrawables)
  {
    try
    {
      const TransformComponent *transform = drawable->getTransform();
      if (!transform)
        continue; // Skip if no transform
      Vector3 objectPos = transform->getPosition();
      float32 distance = (objectPos - cameraPos).Length();
      drawables.emplace_back(distance, drawable);
    }
    catch (const std::runtime_error &)
    {
      // Skip drawables without valid transform components
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

void Scene::invalidateComponentCaches()
{
  _componentCachesDirty = true;
}

void Scene::rebuildComponentCaches() const
{
  if (!_componentCachesDirty)
    return;

  // Clear existing caches
  _cachedCameras.clear();
  _cachedLights.clear();
  _cachedDrawables.clear();

  // Rebuild caches by traversing scene once from root objects only
  // (collectComponentsRecursive will handle children)
  for (const auto &gameObject : _gameObjects)
  {
    if (gameObject->getParent() == nullptr)
    {
      collectComponentsRecursive<CameraComponent>(*gameObject, _cachedCameras);
      collectComponentsRecursive<LightComponent>(*gameObject, _cachedLights);
      collectComponentsRecursive<DrawableComponent>(*gameObject, _cachedDrawables);
    }
  }

  _componentCachesDirty = false;
}

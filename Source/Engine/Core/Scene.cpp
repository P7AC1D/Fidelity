#include "Scene.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "../UI/ImGui/imgui.h"
#include "../Utility/ModelLoader.hpp"
#include "../Rendering/Camera.h"
#include "../Rendering/DeferredRenderer.h"
#include "../Rendering/DebugRenderer.h"
#include "../Rendering/Drawable.h"
#include "../Rendering/Light.h"
#include "../Rendering/ShadowMapRenderer.h"
#include "../RenderApi/RenderDevice.hpp"
#include "GameObject.h"
#include "SceneGraph.h"

static int64 SELECTED_GAME_OBJECT_INDEX = -1;

Scene::Scene() : _objectAddedToScene(false) {}

Scene::~Scene() {}

bool Scene::init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice)
{
  _sceneGraph.reset(new SceneGraph());
  createGameObject("root");

  _renderDevice = renderDevice;
  _deferredRenderer.reset(new DeferredRenderer(windowDims));
  _debugRenderer.reset(new DebugRenderer());
  _shadowMapRenderer.reset(new ShadowMapRenderer());
  return _deferredRenderer->init(_renderDevice) && _debugRenderer->init(_renderDevice) && _shadowMapRenderer->init(renderDevice);
}

GameObject &Scene::createGameObject(const std::string &name)
{
  static uint64 index = 0;

  _sceneGraph->addNode(index);
  _gameObjects.insert(std::pair<uint64, GameObject>(index, std::move(GameObject(name, index))));
  _objectAddedToScene = true;
  return _gameObjects[index++];
}

void Scene::addChildToNode(GameObject &parent, GameObject &child)
{
  _sceneGraph->addChildToNode(parent.getIndex(), child.getIndex());
  parent.addChildNode(child);
}

void Scene::update(float64 dt)
{
  for (auto &gameObject : _gameObjects)
  {
    gameObject.second.update(dt);
  }

  for (auto componentType : _components)
  {
    for (auto &component : _components[componentType.first])
    {
      component->update(dt);
    }
  }
}

void Scene::drawFrame()
{
  if (_renderDevice == nullptr || _deferredRenderer == nullptr)
  {
    std::cerr << "Renderer not initialized." << std::endl;
    return;
  }

  if (_objectAddedToScene)
  {
    calcSceneExtents();
  }

  auto compare = [](DrawableSortMap a, DrawableSortMap b)
  { return a.DistanceToCamera > b.DistanceToCamera; };
  std::multiset<DrawableSortMap, decltype(compare)> culledDrawables(compare);

  auto drawableFindIter = _components.find(ComponentType::Drawable);
  if (drawableFindIter == _components.end())
  {
    return;
  }

  auto cameraFindIter = _components.find(ComponentType::Camera);
  if (cameraFindIter == _components.end())
  {
    return;
  }

  std::shared_ptr<Camera> camera(std::static_pointer_cast<Camera>(cameraFindIter->second[0]));
  std::vector<std::shared_ptr<Drawable>>
      aabbDrawables;
  for (auto component : drawableFindIter->second)
  {
    auto drawable = std::dynamic_pointer_cast<Drawable>(component);
    if (camera->contains(drawable->getAabb()))
    {
      culledDrawables.insert(DrawableSortMap(camera->distanceFrom(drawable->getPosition()), drawable));
    }

    if (drawable->shouldDrawAabb())
    {
      aabbDrawables.push_back(drawable);
    }
  }

  std::vector<std::shared_ptr<Light>> lights;
  for (auto component : _components.find(ComponentType::Light)->second)
  {
    auto light = std::dynamic_pointer_cast<Light>(component);
    lights.push_back(light);
  }

  std::vector<std::shared_ptr<Drawable>> drawables;
  for (const auto &culledDrawable : culledDrawables)
  {
    drawables.push_back(culledDrawable.ComponentPtr);
  }

  _shadowMapRenderer->drawFrame(_renderDevice, drawables, lights, camera, _sceneMaxExtents, _sceneMinExtents);
  _deferredRenderer->drawFrame(_renderDevice, aabbDrawables, drawables, lights, _shadowMapRenderer->getShadowMapRto(), _shadowMapRenderer->getShadowBuffer(), camera);
  _debugRenderer->drawFrame(_renderDevice, _deferredRenderer->getGbuffer(), _deferredRenderer->getShadowRto(), _deferredRenderer->getLightingBuffer(), _shadowMapRenderer->getShadowMapRto(), _deferredRenderer->getSsaoRto(), aabbDrawables, camera);
}

void Scene::drawDebugUi()
{
  ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvail().x, 300), false, ImGuiWindowFlags_HorizontalScrollbar);
  drawSceneGraphUi(0);
  drawGameObjectInspector(SELECTED_GAME_OBJECT_INDEX);
  ImGui::EndChild();

  _shadowMapRenderer->drawDebugUi();
  _deferredRenderer->drawDebugUi();
  _debugRenderer->drawDebugUi();

  ImGui::Separator();
  {
    std::vector<RenderPassTimings> renderPassTimings;
    std::vector<RenderPassTimings> shadowRenderPassTimings(_shadowMapRenderer->getRenderPassTimings());
    std::vector<RenderPassTimings> deferredRenderPassTimings(_deferredRenderer->getRenderPassTimings());
    renderPassTimings.insert(renderPassTimings.end(), shadowRenderPassTimings.begin(), shadowRenderPassTimings.end());
    renderPassTimings.insert(renderPassTimings.end(), deferredRenderPassTimings.begin(), deferredRenderPassTimings.end());

    if (ImGui::CollapsingHeader("Frame Profiler"))
    {
      float32 totalDuration = 0.0;
      for (auto &timings : renderPassTimings)
      {
        float32 duration = static_cast<float32>(timings.Duration) * 1e-6f;
        ImGui::Text("%s: (%.3f ms)", timings.Name.c_str(), duration);
        totalDuration += duration;
      }
      ImGui::Text("All: (%.3f ms)", totalDuration);
    }
  }
}

void Scene::drawSceneGraphUi(int64 nodeIndex)
{
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SELECTED_GAME_OBJECT_INDEX == nodeIndex ? ImGuiTreeNodeFlags_Selected : 0);

  GameObject &gameObject = _gameObjects[nodeIndex];

  if (!_sceneGraph->doesNodeHaveChildren(nodeIndex))
  {
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    ImGui::TreeNodeEx(gameObject.getName().c_str(), flags);
    if (ImGui::IsItemClicked())
    {
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, false);
      SELECTED_GAME_OBJECT_INDEX = nodeIndex;
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, true);
    }
  }
  else
  {
    bool open = ImGui::TreeNodeEx(gameObject.getName().c_str(), flags);
    if (ImGui::IsItemClicked())
    {
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, false);
      SELECTED_GAME_OBJECT_INDEX = nodeIndex;
      setAabbDrawOnGameObject(SELECTED_GAME_OBJECT_INDEX, true);
    }
    if (open)
    {
      for (const auto &childNodeIndex : _sceneGraph->getNodeChildren(nodeIndex))
      {
        drawSceneGraphUi(childNodeIndex);
      }
      ImGui::TreePop();
    }
  }
}

void Scene::drawGameObjectInspector(int64 selectedGameObjectIndex)
{
  if (selectedGameObjectIndex == -1)
  {
    return;
  }

  ImGui::Begin("Inspector", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

  GameObject &gameObject = _gameObjects[selectedGameObjectIndex];
  gameObject.drawInspector();

  ImVec2 screenSize = ImGui::GetIO().DisplaySize;
  auto windowPos = ImVec2(screenSize.x - ImGui::GetWindowWidth(), 0);
  ImGui::SetWindowPos(windowPos);
  ImGui::End();
}

void Scene::setAabbDrawOnGameObject(int64 gameObjectIndex, bool enableAabbDraw)
{
  if (gameObjectIndex == -1)
  {
    return;
  }

  GameObject &gameObject = _gameObjects[gameObjectIndex];
  if (gameObject.hasComponent<Drawable>())
  {
    gameObject.getComponent<Drawable>().enableDrawAabb(enableAabbDraw);
  }
}

void Scene::calcSceneExtents()
{
  auto findIter = _components.find(ComponentType::Drawable);
  if (findIter == _components.end())
  {
    return;
  }

  Vector3 max(std::numeric_limits<float32>::min());
  Vector3 min(std::numeric_limits<float32>::max());
  for (const auto &component : findIter->second)
  {
    auto drawable = std::static_pointer_cast<Drawable>(component);
    const Aabb &aabb = drawable->getAabb();
    min = Math::Min(min, aabb.GetNegBounds());
    max = Math::Max(max, aabb.GetPosBounds());
  }

  _sceneMaxExtents = std::move(max);
  _sceneMinExtents = std::move(min);
  _objectAddedToScene = false;
}
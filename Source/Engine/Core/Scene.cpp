#include "Scene.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "../UI/ImGui/imgui.h"
#include "../Utility/ModelLoader.hpp"
#include "../Rendering/DeferredRenderer.h"
#include "../Rendering/Drawable.h"
#include "../Rendering/Light.h"
#include "../RenderApi/RenderDevice.hpp"
#include "GameObject.h"

static uint64 SELECTED_GAME_OBJECT_INDEX = -1;

bool Scene::init(const Vector2I &windowDims, std::shared_ptr<RenderDevice> renderDevice)
{
  createGameObject("root");

  _renderDevice = renderDevice;
  _deferredRenderer.reset(new DeferredRenderer(windowDims));
  return _deferredRenderer->init(_renderDevice);
}

GameObject &Scene::createGameObject(const std::string &name)
{
  static uint64 index = 0;

  _sceneGraph.insert(std::pair<uint64, std::vector<uint64>>(index, {}));
  _gameObjects.insert(std::pair<uint64, GameObject>(index, std::move(GameObject(name, index))));
  return _gameObjects[index++];
}

void Scene::addChildToNode(GameObject &parent, GameObject &child)
{
  _sceneGraph[parent.getIndex()].push_back(child.getIndex());
}

void Scene::update(float64 dt)
{
  _camera.update(dt);

  for (auto componentType : _components)
  {
    for (auto &component : _components[componentType.first])
    {
      component->update(dt);
    }
  }

  for (auto &gameObject : _gameObjects)
  {
    gameObject.second.update(dt);
  }
}

void Scene::drawFrame() const
{
  if (_renderDevice == nullptr || _deferredRenderer == nullptr)
  {
    std::cerr << "Renderer not initialized." << std::endl;
    return;
  }

  auto compare = [](DrawableSortMap a, DrawableSortMap b)
  { return a.DistanceToCamera < b.DistanceToCamera; };
  std::multiset<DrawableSortMap, decltype(compare)> culledDrawables(compare);

  auto findIter = _components.find(ComponentType::Drawable);
  if (findIter == _components.end())
  {
    return;
  }

  std::vector<std::shared_ptr<Drawable>> aabbDrawables;
  for (auto component : findIter->second)
  {
    auto drawable = std::dynamic_pointer_cast<Drawable>(component);
    if (_camera.intersectsFrustrum(drawable->getAabb()))
    {
      culledDrawables.insert(DrawableSortMap(_camera.distanceFrom(drawable->getPosition()), drawable));      
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
  for (auto culledDrawable : _components.find(ComponentType::Drawable)->second)
  {
    drawables.push_back(std::dynamic_pointer_cast<Drawable>(culledDrawable));
  }

  _deferredRenderer->drawFrame(_renderDevice, aabbDrawables, drawables, lights, _camera);
}

void Scene::drawInspector()
{
  ImGui::BeginChild("SceneGraph", ImVec2(ImGui::GetContentRegionAvail().x, 200), false, ImGuiWindowFlags_HorizontalScrollbar);
  drawInspectorNode(0);
  drawGameObjectInspector(SELECTED_GAME_OBJECT_INDEX);
  ImGui::EndChild();
}

void Scene::drawInspectorNode(uint64 nodeIndex)
{
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (SELECTED_GAME_OBJECT_INDEX == nodeIndex ? ImGuiTreeNodeFlags_Selected : 0);

  GameObject &gameObject = _gameObjects[nodeIndex];

  if (_sceneGraph[nodeIndex].empty())
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
      for (auto childNodeIndex : _sceneGraph[nodeIndex])
      {
        drawInspectorNode(childNodeIndex);
      }
      ImGui::TreePop();
    }
  }
}

void Scene::drawGameObjectInspector(uint64 selectedGameObjectIndex)
{
  if (selectedGameObjectIndex == -1)
  {
    return;
  }

  ImGui::Begin("Inspector", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

  GameObject& gameObject = _gameObjects[selectedGameObjectIndex];
  gameObject.drawInspector();

  ImVec2 screenSize = ImGui::GetIO().DisplaySize;
  auto windowPos = ImVec2(screenSize.x - ImGui::GetWindowWidth(), 0);
  ImGui::SetWindowPos(windowPos);
  ImGui::End();
}

void Scene::setAabbDrawOnGameObject(uint64 gameObjectIndex, bool enableAabbDraw)
{
  GameObject& gameObject = _gameObjects[gameObjectIndex];
  if (gameObject.hasComponent<Drawable>())
  {
    gameObject.getComponent<Drawable>().enableDrawAabb(enableAabbDraw);
  }
}
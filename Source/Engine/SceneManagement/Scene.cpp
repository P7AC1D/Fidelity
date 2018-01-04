#include "Scene.h"

#include "../Rendering/CubeMap.h"
#include "OrbitalCamera.h"
#include "SceneNode.h"

using namespace Rendering;

namespace SceneManagement
{
Scene::Scene(std::string name) :
  _name(std::move(name)),
  _rootNode(new SceneNode("root"))
{}

Scene::~Scene()
{}

void Scene::Update()
{
  _rootNode->Update();
}

void Scene::SetSkyBox(std::shared_ptr<CubeMap> skyBox)
{
  _skyBox = skyBox;
}

std::shared_ptr<SceneNode> Scene::GetRootNode()
{
  return _rootNode;
}
}
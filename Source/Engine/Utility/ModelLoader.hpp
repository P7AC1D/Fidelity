#pragma once
#include <memory>
#include <string>

#include "../Core/Scene.h"

class GameObject;

class ModelLoader
{
public:
  static std::shared_ptr<GameObject> fromFile(Scene &scene, const std::string &filePath, bool reconstructWorldTransforms);
};

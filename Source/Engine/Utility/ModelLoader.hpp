#pragma once
#include <memory>
#include <string>

#include "../Core/Scene.h"

class SceneNode;

class ModelLoader
{
public:
  static void LoadFromFile(Scene &scene, const std::string &filePath, bool reconstructWorldTransforms);
};

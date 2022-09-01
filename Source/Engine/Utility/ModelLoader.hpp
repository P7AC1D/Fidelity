#pragma once
#include <memory>
#include <string>

#include "../Core/Scene.h"

class ModelLoader
{
public:
  static void FromFile(Scene &scene, const std::string &filePath, bool reconstructWorldTransforms);
};

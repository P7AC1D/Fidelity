#pragma once
#include <memory>
#include <string>

class SceneNode;

class ModelLoader
{
public:
  static std::shared_ptr<SceneNode> LoadFromFile(const std::string& filePath);
};

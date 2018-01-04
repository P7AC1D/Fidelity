#pragma once
#include <memory>
#include <vector>

#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"
#include "Scene.h"

namespace Components
{
class Transform;
}

namespace SceneManagement
{
class WorldObject;

class SceneNode
{
public:
  SceneNode(std::string name);
  ~SceneNode();

  void Update();

  std::vector<std::shared_ptr<WorldObject>> GetAllObjectsWithComponent(const std::string& componentName);

  void AddChild(std::shared_ptr<SceneNode> childNode);
  void AddObject(std::shared_ptr<WorldObject> object);

  void SetPosition(const Vector3& position);
  void SetRotation(const Quaternion& rotation);
  void SetScale(const Vector3& scale);

  void Rotate(const Quaternion& rotationDelta);

  std::shared_ptr<SceneNode> CreateChild(const std::string& name);
  std::shared_ptr<WorldObject> CreateObject(const std::string& name);

private:
  SceneNode(SceneNode&) = delete;
  SceneNode(SceneNode&&) = delete;
  SceneNode& operator=(SceneNode&) = delete;
  SceneNode& operator=(SceneNode&&) = delete;

  void UpdateTransforms();
  void UpdateChildObjectsTransforms();

  std::string _name;
  bool _isDirty;
  std::vector<std::shared_ptr<WorldObject>> _objects;
  std::vector<std::shared_ptr<SceneNode>> _childNodes;
  std::shared_ptr<Components::Transform> _absoluteTransform;
  std::shared_ptr<Components::Transform> _localTransform;
  SceneNode* _parentNode;
};
}
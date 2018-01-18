#pragma once
#include <memory>
#include <unordered_map>
#include <string>

#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"

namespace Components
{
class Component;
class Transform;
}

class WorldObject
{
public:
  WorldObject(const std::string& name);
  ~WorldObject();

  void Update();

  void AddComponent(std::shared_ptr<Components::Component> component);

  bool HasComponent(const std::string& name) const;

  std::shared_ptr<Components::Component> GetComponent(const std::string& name);
  Matrix4 GetTransform() const;

  void SetPosition(const Vector3& position);
  void SetScale(const Vector3& scale);
  void SetRotation(const Quaternion& rotation);

  void Rotate(const Quaternion& rotationDelta);

private:
  WorldObject(WorldObject&) = delete;
  WorldObject(WorldObject&&) = delete;
  WorldObject& operator=(WorldObject&) = delete;
  WorldObject& operator=(WorldObject&&) = delete;

  std::string _name;
  bool _isDirty;
  std::shared_ptr<Components::Transform> _localTransform;
  std::shared_ptr<Components::Transform> _absoluteTransform;
  std::unordered_map<std::string, std::shared_ptr<Components::Component>> _components;

  friend class SceneNode;
};
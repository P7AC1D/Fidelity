#pragma once
#include <memory>
#include <unordered_map>
#include <string>

#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"

class Transform;

namespace Components
{
class Component;
}

namespace Rendering
{
class Renderable;
}

class WorldObject
{
public:
  WorldObject(const std::string& name);
  ~WorldObject();

  inline std::shared_ptr<Rendering::Renderable> GetRenderable() { return _renderable; }
  inline std::shared_ptr<Transform> GetTransform() { return _transform; }
  inline void SetParent(WorldObject* parentObject) { _parent = parentObject; }
  inline void SetRenderable(const std::shared_ptr<Rendering::Renderable>& renderable) { _renderable = renderable; }

  void Update();

  void AttachChild(WorldObject* childObject);

  void AddComponent(std::shared_ptr<Components::Component> component);
  bool HasComponent(const std::string& name) const;
  std::shared_ptr<Components::Component> GetComponent(const std::string& name) const;  

  void SetPosition(const Vector3& position);
  void SetScale(const Vector3& scale);
  void SetRotation(const Quaternion& rotation);

  void Rotate(const Quaternion& rotationDelta);

private:
  bool _isDirty;
  std::string _name;
  std::unordered_map<std::string, std::shared_ptr<Components::Component>> _components;
  std::shared_ptr<Rendering::Renderable> _renderable;
  std::shared_ptr<Transform> _transform;
  WorldObject* _parent;
};
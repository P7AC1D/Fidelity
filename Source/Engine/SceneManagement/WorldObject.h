#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"

class Component;
class Renderable;
class Transform;

class WorldObject
{
public:
  WorldObject(const std::string& name);

  inline const std::shared_ptr<Renderable>& GetRenderable() { return _renderable; }
  inline const std::shared_ptr<Transform>& GetTransform() { return _transform; }
  inline void AttachParent(WorldObject* parentObject) { _parent = parentObject; }
  inline void AttachRenderable(const std::shared_ptr<Renderable>& renderable) { _renderable = renderable; }

  void Update();

  void AttachChild(WorldObject* childObject);

  void AddComponent(std::shared_ptr<Component> component);
  bool HasComponent(const std::string& name) const;
  std::shared_ptr<Component> GetComponent(const std::string& name) const;  

  void SetPosition(const Vector3& position);
  void SetScale(const Vector3& scale);
  void SetRotation(const Quaternion& rotation);

  void Rotate(const Quaternion& rotationDelta);

private:
  bool _isDirty;
  std::string _name;
  std::unordered_map<std::string, std::shared_ptr<Component>> _components;
  std::shared_ptr<Renderable> _renderable;
  std::shared_ptr<Transform> _transform;
  WorldObject* _parent;
};

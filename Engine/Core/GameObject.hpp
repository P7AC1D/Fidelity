#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Maths/Matrix4.hpp"
#include "../Maths/Quaternion.hpp"
#include "../Maths/Vector3.hpp"
#include "Types.h"

class Component;

class GameObject
{
public:
  GameObject(const std::string& name);
  
  void AddComponent(Component&& component);
  const Component& GetComponent(const std::string& componentType);
  
  void SetPosition(const Vector3& position);
  void SetScale(const Vector3& scale);
  void SetOrientation(const Quaternion& orientation);
  
  inline Vector3 GetPosition() const { return _position; }
  inline Vector3 GetScale() const { return _scale; }
  inline Quaternion GetOrientation() const { return _orientation; }
  Matrix4 GetTransform();
  
  virtual void OnUpdate() const {}
  
  inline std::string GetName() const { return _name; }
  inline uint64 GetId() const { return _id; }
  
protected:
  void Update(uint32 dtMs) const;
  
private:
  std::vector<Component> _components;
  std::string _name;
  uint64 _id;
  Vector3 _position;
  Vector3 _scale;
  Quaternion _orientation;
  Matrix4 _transform;
  bool _needsUpdate;
  
  friend class SceneManager;
};

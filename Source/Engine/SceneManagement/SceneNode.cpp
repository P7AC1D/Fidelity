#include "SceneNode.h"

#include "../Components/Transform.h"

#include "WorldObject.h"

using namespace Components;

namespace SceneManagement
{
SceneNode::SceneNode(std::string name) :
  _name(name),
  _isDirty(true),
  _absoluteTransform(new Transform),
  _localTransform(new Transform),
  _parentNode(nullptr)
{}

SceneNode::~SceneNode()
{
  _parentNode = nullptr;
}

void SceneNode::Update()
{
  UpdateTransforms();
  for (auto& node : _childNodes)
  {
    node->Update();
  }
  for (auto& object : _objects)
  {
    object->Update();
  }
}

std::vector<std::shared_ptr<WorldObject>> SceneNode::GetAllObjectsWithComponent(const std::string& componentName)
{
  std::vector<std::shared_ptr<WorldObject>> objects;
  for (auto& object : _objects)
  {
    if (object->GetComponent(componentName))
    {
      objects.push_back(object);
    }
  }
  for (auto& node : _childNodes)
  {
    auto childNodeObjects = node->GetAllObjectsWithComponent(componentName);
    objects.insert(objects.end(), childNodeObjects.begin(), childNodeObjects.end());
  }
  return std::move(objects);
}

void SceneNode::AddChild(std::shared_ptr<SceneNode> childNode)
{
  childNode->_parentNode = this;
  _childNodes.push_back(childNode);
}

void SceneNode::AddObject(std::shared_ptr<WorldObject> object)
{
  _objects.push_back(object);
}

void SceneNode::SetPosition(const Vector3& position)
{
  _localTransform->SetPosition(position);
  _isDirty = true;
}

void SceneNode::SetRotation(const Quaternion& rotation)
{
  _localTransform->SetRotation(rotation);
  _isDirty = true;
}

void SceneNode::SetScale(const Vector3& scale)
{
  _localTransform->SetScale(scale);
  _isDirty = true;
}

void SceneNode::Rotate(const Quaternion& rotationDelta)
{
  _localTransform->Rotate(rotationDelta);
  _isDirty = true;
}

std::shared_ptr<SceneNode> SceneNode::CreateChild(const std::string& name)
{
  auto newChild = std::make_shared<SceneNode>(name);
  newChild->_parentNode = this;
  AddChild(newChild);
  return newChild;
}

std::shared_ptr<WorldObject> SceneNode::CreateObject(const std::string& name)
{
  auto newObject = std::make_shared<WorldObject>(name);
  AddObject(newObject);
  return newObject;
}

void SceneNode::UpdateTransforms()
{
  // Root node has no parent
  if (!_parentNode)
  {
    _absoluteTransform = _localTransform;
  }
  else
  {
    auto origin = _parentNode->_absoluteTransform->GetPosition();
    auto orientation = _parentNode->_absoluteTransform->GetRotation();
    auto point = _parentNode->_localTransform->GetPosition();

    _absoluteTransform->SetPosition(origin + orientation.Rotate(point - origin));
    _absoluteTransform->SetScale(_parentNode->_absoluteTransform->GetScale() + _localTransform->GetScale());
    _absoluteTransform->SetRotation(Quaternion::Normalize(_localTransform->GetRotation() * orientation));
  }

  UpdateChildObjectsTransforms();
}

void SceneNode::UpdateChildObjectsTransforms()
{
  for (auto& object : _objects)
  {
    auto origin = _absoluteTransform->GetPosition();
    auto orientation = _absoluteTransform->GetRotation();
    auto point = object->_localTransform->GetPosition();

    object->_absoluteTransform->SetPosition(origin + orientation.Rotate(point - origin));
    object->_absoluteTransform->SetScale(_absoluteTransform->GetScale() + object->_localTransform->GetScale());
    object->_absoluteTransform->SetRotation(Quaternion::Normalize(object->_localTransform->GetRotation() * orientation));
  }
}
}

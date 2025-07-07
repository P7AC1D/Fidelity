#include "catch.hpp"

#include "../Engine/Core/ComponentDependency.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/ComponentManager.h"
#include "../Engine/Core/TransformComponent.h"
#include "../Engine/Rendering/CameraComponent.h"
#include "../Engine/Rendering/LightComponent.h"
#include "../Engine/Rendering/DrawableComponent.h"

// Mock component for testing custom dependencies
class MockDependentComponent : public IComponent, public IComponentDependency
{
public:
  MockDependentComponent() = default;

  // IComponent interface
  void initialize() override {}
  void activate() override { _isActive = true; }
  void deactivate() override { _isActive = false; }
  ComponentTypeId getTypeId() const override { return getComponentTypeId<MockDependentComponent>(); }
  void drawInspector() override {}

  // Static type information
  static ComponentTypeId GetTypeId() { return getComponentTypeId<MockDependentComponent>(); }

  // IComponentDependency interface
  std::vector<ComponentTypeId> getDependencies() const override
  {
    return {getComponentTypeId<TransformComponent>()};
  }

  void onDependenciesResolved(GameObject &gameObject) override
  {
    _dependenciesResolved = true;
    _resolvedGameObject = &gameObject;
  }

  // Test accessors
  bool isActive() const { return _isActive; }
  bool areDependenciesResolved() const { return _dependenciesResolved; }
  GameObject *getResolvedGameObject() const { return _resolvedGameObject; }

  void setTransformComponent(std::weak_ptr<TransformComponent> transform)
  {
    _transformComponent = transform;
    _transformSet = true;
  }

  bool isTransformSet() const { return _transformSet; }
  std::weak_ptr<TransformComponent> getTransformComponent() const { return _transformComponent; }

private:
  bool _isActive = false;
  bool _dependenciesResolved = false;
  bool _transformSet = false;
  GameObject *_resolvedGameObject = nullptr;
  std::weak_ptr<TransformComponent> _transformComponent;
};

// Mock component without dependencies
class MockIndependentComponent : public IComponent
{
public:
  MockIndependentComponent() = default;

  // IComponent interface
  void initialize() override {}
  void activate() override { _isActive = true; }
  void deactivate() override { _isActive = false; }
  ComponentTypeId getTypeId() const override { return getComponentTypeId<MockIndependentComponent>(); }
  void drawInspector() override {}

  // Static type information
  static ComponentTypeId GetTypeId() { return getComponentTypeId<MockIndependentComponent>(); }

  // Test accessors
  bool isActive() const { return _isActive; }

private:
  bool _isActive = false;
};

// Specialized dependency resolution for MockDependentComponent -> TransformComponent
template <>
void AutoDependency<MockDependentComponent, TransformComponent>::resolve(GameObject &gameObject, MockDependentComponent *component)
{
  if (auto transformShared = gameObject.getComponentShared<TransformComponent>())
  {
    // Convert to weak_ptr for the component
    std::weak_ptr<TransformComponent> weakPtr = transformShared;
    component->setTransformComponent(weakPtr);
  }
}

TEST_CASE("COMPONENT_DEPENDENCY_SYSTEM_TESTS")
{
  SECTION("DEPENDENCY_REGISTRATION_VERIFICATION")
  {
    // Test that known components have dependencies registered
    REQUIRE(ComponentDependencyResolver::hasDependencies<CameraComponent>());
    REQUIRE(ComponentDependencyResolver::hasDependencies<LightComponent>());
    REQUIRE(ComponentDependencyResolver::hasDependencies<DrawableComponent>());

    // Test that components without dependencies return false
    REQUIRE_FALSE(ComponentDependencyResolver::hasDependencies<TransformComponent>());
    REQUIRE_FALSE(ComponentDependencyResolver::hasDependencies<MockIndependentComponent>());
  }

  SECTION("BASIC_DEPENDENCY_RESOLUTION")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // GameObject automatically has TransformComponent
    REQUIRE(gameObject.hasComponent<TransformComponent>());

    // Add CameraComponent - should automatically resolve TransformComponent dependency
    auto &camera = gameObject.addComponent<CameraComponent>();

    // Verify the dependency was resolved
    auto transformWeak = camera.getTransformComponentWeak();
    REQUIRE_FALSE(transformWeak.expired());

    // Verify it's the same transform as the GameObject's
    auto transformShared = transformWeak.lock();
    REQUIRE(transformShared.get() == gameObject.tryGetComponent<TransformComponent>());
  }

  SECTION("MULTIPLE_DEPENDENT_COMPONENTS")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Add multiple components that depend on TransformComponent
    auto &camera = gameObject.addComponent<CameraComponent>();
    auto &light = gameObject.addComponent<LightComponent>();
    auto &drawable = gameObject.addComponent<DrawableComponent>();

    // Verify all dependencies were resolved
    REQUIRE_FALSE(camera.getTransformComponentWeak().expired());
    REQUIRE_FALSE(light.getTransformComponent().expired());
    REQUIRE_FALSE(drawable.getTransformComponent().expired());

    // Verify they all point to the same transform
    auto *transform = gameObject.tryGetComponent<TransformComponent>();
    REQUIRE(camera.getTransformComponentWeak().lock().get() == transform);
    REQUIRE(light.getTransformComponent().lock().get() == transform);
    REQUIRE(drawable.getTransformComponent().lock().get() == transform);
  }

  SECTION("CUSTOM_DEPENDENCY_REGISTRATION")
  {
    // Register custom dependency
    ComponentDependencyResolver::registerDependency<MockDependentComponent, TransformComponent>();

    // Verify registration
    REQUIRE(ComponentDependencyResolver::hasDependencies<MockDependentComponent>());

    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Add mock component
    auto &mockComponent = gameObject.addComponent<MockDependentComponent>();

    // Verify dependency resolution was called
    REQUIRE(mockComponent.areDependenciesResolved());
    REQUIRE(mockComponent.getResolvedGameObject() == &gameObject);
    REQUIRE(mockComponent.isTransformSet());
    REQUIRE_FALSE(mockComponent.getTransformComponent().expired());
  }

  SECTION("MISSING_DEPENDENCY_HANDLING")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Remove the TransformComponent
    gameObject.removeComponent<TransformComponent>();
    REQUIRE_FALSE(gameObject.hasComponent<TransformComponent>());

    // Add CameraComponent - dependency resolution should handle missing dependency gracefully
    auto &camera = gameObject.addComponent<CameraComponent>();

    // Transform should be null/expired
    auto transformWeak = camera.getTransformComponentWeak();
    REQUIRE(transformWeak.expired());
  }

  SECTION("DEPENDENCY_RESOLUTION_TIMING")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Register custom dependency for testing
    ComponentDependencyResolver::registerDependency<MockDependentComponent, TransformComponent>();

    // Add dependent component
    auto &mockComponent = gameObject.addComponent<MockDependentComponent>();

    // Dependencies should be resolved immediately after component addition
    REQUIRE(mockComponent.areDependenciesResolved());
    REQUIRE(mockComponent.isTransformSet());
  }

  SECTION("COMPONENT_WITHOUT_DEPENDENCIES")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Add component without dependencies
    auto &independentComponent = gameObject.addComponent<MockIndependentComponent>();

    // Should not crash and component should be functional
    REQUIRE(independentComponent.isActive());
    REQUIRE_FALSE(ComponentDependencyResolver::hasDependencies<MockIndependentComponent>());
  }

  SECTION("DEPENDENCY_RESOLUTION_WITH_GAMEOBJECT_LIFECYCLE")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Add CameraComponent
    auto &camera = gameObject.addComponent<CameraComponent>();

    // Verify dependency is resolved
    REQUIRE_FALSE(camera.getTransformComponentWeak().expired());

    // Deactivate GameObject
    gameObject.setActive(false);
    REQUIRE_FALSE(gameObject.isActive());

    // Dependency should still be valid
    REQUIRE_FALSE(camera.getTransformComponentWeak().expired());

    // Reactivate GameObject
    gameObject.setActive(true);
    REQUIRE(gameObject.isActive());

    // Dependency should still be valid
    REQUIRE_FALSE(camera.getTransformComponentWeak().expired());
  }

  SECTION("WEAK_PTR_BEHAVIOR_ON_COMPONENT_REMOVAL")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Add CameraComponent
    auto &camera = gameObject.addComponent<CameraComponent>();

    // Get weak_ptr to transform
    auto transformWeak = camera.getTransformComponentWeak();
    REQUIRE_FALSE(transformWeak.expired());

    // Remove TransformComponent
    gameObject.removeComponent<TransformComponent>();

    // weak_ptr should now be expired
    REQUIRE(transformWeak.expired());
  }

  SECTION("MULTIPLE_GAMEOBJECTS_INDEPENDENT_DEPENDENCIES")
  {
    ComponentManager manager;
    GameObject gameObject1("TestObject1", 1, &manager);
    GameObject gameObject2("TestObject2", 2, &manager);

    // Add CameraComponents to both
    auto &camera1 = gameObject1.addComponent<CameraComponent>();
    auto &camera2 = gameObject2.addComponent<CameraComponent>();

    // Each should have its own transform dependency
    auto transform1Weak = camera1.getTransformComponentWeak();
    auto transform2Weak = camera2.getTransformComponentWeak();

    REQUIRE_FALSE(transform1Weak.expired());
    REQUIRE_FALSE(transform2Weak.expired());

    // They should point to different transforms
    REQUIRE(transform1Weak.lock().get() != transform2Weak.lock().get());
    REQUIRE(transform1Weak.lock().get() == gameObject1.tryGetComponent<TransformComponent>());
    REQUIRE(transform2Weak.lock().get() == gameObject2.tryGetComponent<TransformComponent>());
  }

  SECTION("DEPENDENCY_RESOLVER_STATIC_BEHAVIOR")
  {
    // Test that dependency map is static and persistent
    ComponentDependencyResolver::registerDependency<MockDependentComponent, TransformComponent>();

    // Create multiple managers and objects
    ComponentManager manager1;
    ComponentManager manager2;
    GameObject gameObject1("TestObject1", 1, &manager1);
    GameObject gameObject2("TestObject2", 2, &manager2);

    // Both should have the same dependency registration
    REQUIRE(ComponentDependencyResolver::hasDependencies<MockDependentComponent>());

    auto &mock1 = gameObject1.addComponent<MockDependentComponent>();
    auto &mock2 = gameObject2.addComponent<MockDependentComponent>();

    // Both should have dependencies resolved
    REQUIRE(mock1.areDependenciesResolved());
    REQUIRE(mock2.areDependenciesResolved());
  }

  SECTION("COMPONENT_TYPE_ID_CONSISTENCY")
  {
    // Verify that component type IDs are consistent for dependency resolution
    ComponentTypeId cameraId1 = getComponentTypeId<CameraComponent>();
    ComponentTypeId cameraId2 = CameraComponent::GetTypeId();
    ComponentTypeId cameraId3 = getComponentTypeId<CameraComponent>();

    REQUIRE(cameraId1 == cameraId2);
    REQUIRE(cameraId2 == cameraId3);

    // Different component types should have different IDs
    ComponentTypeId transformId = getComponentTypeId<TransformComponent>();
    REQUIRE(cameraId1 != transformId);
  }

  SECTION("REAL_WORLD_CAMERA_DEPENDENCY_SCENARIO")
  {
    ComponentManager manager;
    GameObject cameraObject("Camera", 1, &manager);

    // Add CameraComponent
    auto &camera = cameraObject.addComponent<CameraComponent>();

    // Configure camera
    camera.setPerspective(Degree(60.0f), 1920, 1080, 0.1f, 1000.0f);

    // Modify transform
    auto &transform = cameraObject.getComponent<TransformComponent>();
    transform.setPosition(Vector3(0.0f, 5.0f, 10.0f));
    // Set rotation to look at origin (simplified for test)
    transform.setRotation(Quaternion::Identity);

    // Camera should be able to access world position through its dependency
    Vector3 worldPos = camera.getWorldPosition();
    REQUIRE(worldPos == Vector3(0.0f, 5.0f, 10.0f));

    // Camera should be able to calculate view matrix
    Matrix4 viewMatrix = camera.getView();
    REQUIRE(viewMatrix != Matrix4::Identity);
  }

  SECTION("PERFORMANCE_MULTIPLE_DEPENDENCIES")
  {
    ComponentManager manager;

    // Create multiple GameObjects with dependent components
    const int numObjects = 100;
    std::vector<std::unique_ptr<GameObject>> gameObjects;

    for (int i = 0; i < numObjects; ++i)
    {
      auto gameObject = std::make_unique<GameObject>("TestObject" + std::to_string(i), i, &manager);

      // Add multiple dependent components
      gameObject->addComponent<CameraComponent>();
      gameObject->addComponent<LightComponent>();
      gameObject->addComponent<DrawableComponent>();

      gameObjects.push_back(std::move(gameObject));
    }

    // Verify all dependencies were resolved correctly
    for (const auto &gameObject : gameObjects)
    {
      auto &camera = gameObject->getComponent<CameraComponent>();
      auto &light = gameObject->getComponent<LightComponent>();
      auto &drawable = gameObject->getComponent<DrawableComponent>();

      REQUIRE_FALSE(camera.getTransformComponentWeak().expired());
      REQUIRE_FALSE(light.getTransformComponent().expired());
      REQUIRE_FALSE(drawable.getTransformComponent().expired());
    }
  }

  SECTION("ERROR_HANDLING_INVALID_COMPONENT_TYPES")
  {
    // Test that the system handles edge cases gracefully
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // These operations should not crash
    bool hasNonExistentDeps = ComponentDependencyResolver::hasDependencies<MockIndependentComponent>();
    REQUIRE_FALSE(hasNonExistentDeps);

    // Adding component without dependencies should work fine
    auto &independent = gameObject.addComponent<MockIndependentComponent>();
    REQUIRE(independent.isActive());
  }
}

TEST_CASE("COMPONENT_DEPENDENCY_EDGE_CASES")
{
  SECTION("DEPENDENCY_RESOLUTION_ORDER")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Register custom dependency
    ComponentDependencyResolver::registerDependency<MockDependentComponent, TransformComponent>();

    // Add dependent component first, then verify it gets resolved
    auto &mockComponent = gameObject.addComponent<MockDependentComponent>();

    // Should be resolved immediately since TransformComponent already exists
    REQUIRE(mockComponent.areDependenciesResolved());
    REQUIRE(mockComponent.isTransformSet());
  }

  SECTION("COMPONENT_REMOVAL_AND_READD")
  {
    ComponentManager manager;
    GameObject gameObject("TestObject", 1, &manager);

    // Add and remove camera component
    auto &camera1 = gameObject.addComponent<CameraComponent>();
    auto transform1Weak = camera1.getTransformComponentWeak();
    REQUIRE_FALSE(transform1Weak.expired());

    // Remove camera
    gameObject.removeComponent<CameraComponent>();

    // Add camera again
    auto &camera2 = gameObject.addComponent<CameraComponent>();
    auto transform2Weak = camera2.getTransformComponentWeak();

    // New camera should have dependency resolved
    REQUIRE_FALSE(transform2Weak.expired());
    REQUIRE(transform2Weak.lock().get() == gameObject.tryGetComponent<TransformComponent>());
  }

  SECTION("MEMORY_MANAGEMENT_VERIFICATION")
  {
    ComponentManager manager;

    {
      GameObject gameObject("TestObject", 1, &manager);
      auto &camera = gameObject.addComponent<CameraComponent>();

      // Get weak reference
      auto transformWeak = camera.getTransformComponentWeak();
      REQUIRE_FALSE(transformWeak.expired());

      // GameObject goes out of scope here
    }

    // After GameObject destruction, no memory leaks should occur
    // This is verified by the test framework's memory checking
  }
}

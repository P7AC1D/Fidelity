#include "ShadowFrustum.h"
#include "CameraComponent.h"
#include "DrawableComponent.h"
#include "Material.h"
#include "../Utility/Assert.hpp"

ShadowFrustum::ShadowFrustum() : _cascadeCount(0)
{
}

void ShadowFrustum::buildFromLightTransforms(const std::vector<Matrix4>& lightTransforms, uint32 cascadeCount)
{
    Assert::throwIfFalse(cascadeCount <= MAX_CASCADE_LAYERS, "Cascade count exceeds maximum allowed layers");
    Assert::throwIfFalse(lightTransforms.size() >= cascadeCount, "Not enough light transforms for cascade count");
    
    _cascadeCount = cascadeCount;
    
    for (uint32 i = 0; i < _cascadeCount; ++i)
    {
        _cascadeFrustums[i] = extractFrustumFromMatrix(lightTransforms[i]);
    }
}

void ShadowFrustum::buildExtendedCameraFrustum(const CameraComponent& camera, float32 shadowDistance)
{
    Matrix4 view = camera.getView();
    
    // Check if view matrix is identity (camera not properly initialized)
    if (view == Matrix4::Identity) {
        // Fallback: When camera transforms aren't properly set up, create a simple
        // frustum that includes everything in front of the camera
        // This prevents objects from being incorrectly culled due to bad transforms
        
        float32 extendedFar = camera.getFar() * shadowDistance;
        float32 extendedNear = camera.getNear() * 0.5f;
        
        // Create a very permissive frustum that doesn't cull much
        // Use a simple orthographic-style frustum that covers a large area
        Vector3 center(0.0f, 0.0f, -(extendedFar + extendedNear) * 0.5f);
        float32 size = extendedFar * 2.0f; // Large size to be permissive
        
        // Create planes for a large bounding box
        Plane left(Vector3(1.0f, 0.0f, 0.0f), Vector3(-size, 0.0f, 0.0f));
        Plane right(Vector3(-1.0f, 0.0f, 0.0f), Vector3(size, 0.0f, 0.0f));
        Plane top(Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, size, 0.0f));
        Plane bottom(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, -size, 0.0f));
        Plane near(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -extendedNear));
        Plane far(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -extendedFar));
        
        _extendedCameraFrustum = Frustrum(left, right, top, bottom, near, far);
        return;
    }
    
    // Original matrix-based approach for properly initialized cameras
    float32 extendedFar = camera.getFar() * shadowDistance;
    float32 extendedNear = camera.getNear() * 0.5f;
    
    Matrix4 extendedProj = Matrix4::Perspective(camera.getFov(), camera.getAspectRatio(), extendedNear, extendedFar);
    Matrix4 extendedViewProj = extendedProj * view;
    
    _extendedCameraFrustum = extractFrustumFromMatrix(extendedViewProj);
}

std::vector<std::shared_ptr<DrawableComponent>> ShadowFrustum::cullForCascade(uint32 cascadeIndex,
                                                                     const std::vector<std::shared_ptr<DrawableComponent>>& objects) const
{
    Assert::throwIfFalse(cascadeIndex < _cascadeCount, "Cascade index out of range");
    
    std::vector<std::shared_ptr<DrawableComponent>> culledObjects;
    culledObjects.reserve(objects.size() / 4); // Rough estimate to reduce allocations
    
    const Frustrum& cascadeFrustum = _cascadeFrustums[cascadeIndex];
    
    for (const auto& drawable : objects)
    {
        // Test object's AABB against cascade frustum
        const TransformComponent* transform = drawable->getCachedTransform();
        if (transform && cascadeFrustum.contains(drawable->getAabb(), *transform))
        {
            culledObjects.push_back(drawable);
        }
    }
    
    return culledObjects;
}

std::vector<std::shared_ptr<DrawableComponent>> ShadowFrustum::broadPhaseCull(const std::vector<std::shared_ptr<DrawableComponent>>& objects) const
{
    std::vector<std::shared_ptr<DrawableComponent>> culledObjects;
    culledObjects.reserve(objects.size() / 2); // Rough estimate
    
    for (const auto& drawable : objects)
    {
        // Test against extended camera frustum
        const TransformComponent* transform = drawable->getCachedTransform();
        if (transform && _extendedCameraFrustum.contains(drawable->getAabb(), *transform))
        {
            culledObjects.push_back(drawable);
        }
    }
    
    return culledObjects;
}

std::vector<std::shared_ptr<DrawableComponent>> ShadowFrustum::shadowRelevanceFilter(const std::vector<std::shared_ptr<DrawableComponent>>& objects) const
{
    std::vector<std::shared_ptr<DrawableComponent>> filteredObjects;
    filteredObjects.reserve(objects.size());
    
    for (const auto& drawable : objects)
    {
        // Check if object should cast shadows
        if (shouldCastShadows(drawable))
        {
            filteredObjects.push_back(drawable);
        }
    }
    
    return filteredObjects;
}

const Frustrum& ShadowFrustum::getCascadeFrustum(uint32 cascadeIndex) const
{
    Assert::throwIfFalse(cascadeIndex < _cascadeCount, "Cascade index out of range");
    return _cascadeFrustums[cascadeIndex];
}

const Frustrum& ShadowFrustum::getExtendedCameraFrustum() const
{
    return _extendedCameraFrustum;
}

Frustrum ShadowFrustum::extractFrustumFromMatrix(const Matrix4& transform) const
{
    // Extract frustum planes from the view-projection matrix
    // This uses the standard method of extracting planes from a 4x4 matrix
    
    // Helper function to create plane from equation coefficients
    auto createPlaneFromCoeffs = [](float32 a, float32 b, float32 c, float32 d) -> Plane {
        Vector3 normal(a, b, c);
        float32 length = normal.Length();
        if (length > 0.0f) {
            normal = normal / length;
            d = d / length;
        }
        // Create a point on the plane using the normal and distance
        Vector3 point = normal * (-d);
        return Plane(normal, point);
    };
    
    // Left plane: add 4th column to 1st column
    Plane left = createPlaneFromCoeffs(
        transform[0][3] + transform[0][0],
        transform[1][3] + transform[1][0],
        transform[2][3] + transform[2][0],
        transform[3][3] + transform[3][0]
    );
    
    // Right plane: subtract 1st column from 4th column
    Plane right = createPlaneFromCoeffs(
        transform[0][3] - transform[0][0],
        transform[1][3] - transform[1][0],
        transform[2][3] - transform[2][0],
        transform[3][3] - transform[3][0]
    );
    
    // Bottom plane: add 4th column to 2nd column
    Plane bottom = createPlaneFromCoeffs(
        transform[0][3] + transform[0][1],
        transform[1][3] + transform[1][1],
        transform[2][3] + transform[2][1],
        transform[3][3] + transform[3][1]
    );
    
    // Top plane: subtract 2nd column from 4th column
    Plane top = createPlaneFromCoeffs(
        transform[0][3] - transform[0][1],
        transform[1][3] - transform[1][1],
        transform[2][3] - transform[2][1],
        transform[3][3] - transform[3][1]
    );
    
    // Near plane: add 4th column to 3rd column
    Plane near = createPlaneFromCoeffs(
        transform[0][3] + transform[0][2],
        transform[1][3] + transform[1][2],
        transform[2][3] + transform[2][2],
        transform[3][3] + transform[3][2]
    );
    
    // Far plane: subtract 3rd column from 4th column
    Plane far = createPlaneFromCoeffs(
        transform[0][3] - transform[0][2],
        transform[1][3] - transform[1][2],
        transform[2][3] - transform[2][2],
        transform[3][3] - transform[3][2]
    );
    
    return Frustrum(left, right, top, bottom, near, far);
}

bool ShadowFrustum::shouldCastShadows(const std::shared_ptr<DrawableComponent>& drawable) const
{
    // Check material properties
    auto material = drawable->getMaterial();
    if (!material)
    {
        return false;
    }
    
    // Skip fully transparent objects
    if (material->hasOpacityTexture())
    {
        // For now, we'll still render transparent objects in shadow pass
        // In a more advanced system, we might check opacity values
        return true;
    }
    
    // Skip objects that are too small to cast meaningful shadows
    const auto& aabb = drawable->getAabb();
    Vector3 size = aabb.getExtents() * 2.0f; // getExtents returns half-extents
    float32 minSize = 0.1f; // Minimum size threshold
    
    // Check if AABB is valid (not zero or negative)
    if (size.X <= 0.0f || size.Y <= 0.0f || size.Z <= 0.0f)
    {
        return false; // Invalid or zero-sized AABB
    }
    
    // Check if all dimensions are below the minimum threshold
    if (size.X < minSize && size.Y < minSize && size.Z < minSize)
    {
        return false;
    }
    
    return true;
}

bool ShadowFrustum::isLargeEnoughForShadows(const std::shared_ptr<DrawableComponent>& drawable, const CameraComponent& camera) const
{
    // Calculate approximate screen-space size
    const TransformComponent* transform = drawable->getCachedTransform();
    if (!transform) {
        return false; // No transform, can't calculate size
    }
    Vector3 objectPos = transform->getPosition();
    Vector3 cameraPos = camera.getWorldPosition();
    float32 distance = (objectPos - cameraPos).Length();
    
    // Get object's bounding sphere radius
    const auto& aabb = drawable->getAabb();
    float32 radius = aabb.getExtents().Length(); // Approximate radius

    // Calculate screen-space size (rough approximation)
    float32 screenSize = (radius / distance) * camera.getFov().InRadians();

    // Skip objects that would be smaller than ~2 pixels on screen
    const float32 minScreenSize = 0.002f; // Roughly 2 pixels at 1080p

    return screenSize > minScreenSize;
}

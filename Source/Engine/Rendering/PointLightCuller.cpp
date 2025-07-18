#include "PointLightCuller.h"
#include "Material.h"
#include "../Core/TransformComponent.h"
#include "../Utility/Assert.hpp"
#include <algorithm>

// Static member definitions
const std::array<Vector3, 6> PointLightCuller::FACE_DIRECTIONS = {{
    Vector3( 1, 0, 0), Vector3(-1, 0, 0),  // +X, -X
    Vector3( 0, 1, 0), Vector3( 0,-1, 0),  // +Y, -Y  
    Vector3( 0, 0, 1), Vector3( 0, 0,-1)   // +Z, -Z
}};

const std::array<Vector3, 6> PointLightCuller::FACE_UPS = {{
    Vector3(0,-1, 0), Vector3(0,-1, 0),  // +X, -X
    Vector3(0, 0, 1), Vector3(0, 0,-1),  // +Y, -Y
    Vector3(0,-1, 0), Vector3(0,-1, 0)   // +Z, -Z
}};

PointLightCuller::PointLightCuller()
{
    // Initialize with default settings
}

float PointLightCuller::CullingResult::averageFaceCullingRatio() const
{
    if (sphereCulledCount == 0) return 0.0f;
    
    float totalRatio = 0.0f;
    for (uint32 i = 0; i < 6; ++i)
    {
        totalRatio += (float)faceCulledCounts[i] / sphereCulledCount;
    }
    return totalRatio / 6.0f;
}

PointLightCuller::CullingResult PointLightCuller::cullObjectsForPointLight(
    const std::shared_ptr<LightComponent>& pointLight,
    const std::vector<std::shared_ptr<DrawableComponent>>& objects) const
{
    Assert::throwIfFalse(pointLight->getLightType() == LightComponentType::Point, 
                        "PointLightCuller can only be used with point lights");

    CullingResult result;
    result.originalCount = static_cast<uint32>(objects.size());
    
    Vector3 lightPos = pointLight->getPosition();
    float lightRadius = pointLight->getRadius();

    // Stage 1: Sphere culling (cull objects outside light radius)
    if (_settings.enableSphereCulling)
    {
        result.sphereCulled = sphereCull(lightPos, lightRadius, objects);
    }
    else
    {
        result.sphereCulled = objects; // No sphere culling, use all objects
    }
    
    result.sphereCulledCount = static_cast<uint32>(result.sphereCulled.size());

    // Stage 2: Per-face frustum culling
    if (_settings.enableFaceCulling)
    {
        for (uint32 faceIndex = 0; faceIndex < 6; ++faceIndex)
        {
            result.faceCulled[faceIndex] = faceCull(lightPos, lightRadius, faceIndex, result.sphereCulled);
            result.faceCulledCounts[faceIndex] = static_cast<uint32>(result.faceCulled[faceIndex].size());
        }
    }
    else
    {
        // No face culling, use sphere-culled objects for all faces
        for (uint32 faceIndex = 0; faceIndex < 6; ++faceIndex)
        {
            result.faceCulled[faceIndex] = result.sphereCulled;
            result.faceCulledCounts[faceIndex] = result.sphereCulledCount;
        }
    }

    return result;
}

std::vector<std::shared_ptr<DrawableComponent>> PointLightCuller::sphereCull(
    const Vector3& lightPos, 
    float lightRadius,
    const std::vector<std::shared_ptr<DrawableComponent>>& objects) const
{
    std::vector<std::shared_ptr<DrawableComponent>> culledObjects;
    culledObjects.reserve(objects.size() / 2); // Rough estimate to reduce reallocations

    for (const auto& drawable : objects)
    {
        // Skip objects that shouldn't cast shadows for this light
        if (!shouldCastShadowForLight(drawable, lightPos, lightRadius))
        {
            continue;
        }

        // Get object's world-space AABB
        const Aabb& aabb = drawable->getAabb();
        
        // Test sphere-AABB intersection
        if (sphereIntersectsAABB(lightPos, lightRadius, aabb))
        {
            culledObjects.push_back(drawable);
        }
    }

    return culledObjects;
}

std::vector<std::shared_ptr<DrawableComponent>> PointLightCuller::faceCull(
    const Vector3& lightPos,
    float lightRadius,
    uint32 faceIndex,
    const std::vector<std::shared_ptr<DrawableComponent>>& objects) const
{
    Assert::throwIfFalse(faceIndex < 6, "Face index must be between 0-5");

    std::vector<std::shared_ptr<DrawableComponent>> culledObjects;
    culledObjects.reserve(objects.size() / 3); // Rough estimate for ~120 degree FOV

    // Create frustum for this face
    Frustrum faceFrustum = createFaceFrustum(lightPos, lightRadius, faceIndex);

    for (const auto& drawable : objects)
    {
        const Aabb& aabb = drawable->getAabb();
        
        // Test AABB against face frustum
        // Use identity matrix since getAabb() already returns world-space bounds
        if (faceFrustum.contains(aabb, Matrix4::Identity))
        {
            culledObjects.push_back(drawable);
        }
    }

    return culledObjects;
}

bool PointLightCuller::sphereIntersectsAABB(const Vector3& sphereCenter, float sphereRadius, const Aabb& aabb) const
{
    // Find the closest point on the AABB to the sphere center
    Vector3 aabbMin = aabb.getMin();
    Vector3 aabbMax = aabb.getMax();
    
    // Clamp sphere center to AABB bounds to get closest point
    Vector3 closestPoint;
    closestPoint.X = std::max(aabbMin.X, std::min(sphereCenter.X, aabbMax.X));
    closestPoint.Y = std::max(aabbMin.Y, std::min(sphereCenter.Y, aabbMax.Y));
    closestPoint.Z = std::max(aabbMin.Z, std::min(sphereCenter.Z, aabbMax.Z));
    
    // Calculate squared distance to avoid expensive sqrt
    Vector3 diff = closestPoint - sphereCenter;
    float length = diff.Length();
    float distanceSquared = length * length;
    float radiusSquared = sphereRadius * sphereRadius;
    
    return distanceSquared <= radiusSquared;
}

float PointLightCuller::getObjectDistanceFromLight(const Vector3& lightPos, const Aabb& aabb) const
{
    Vector3 aabbMin = aabb.getMin();
    Vector3 aabbMax = aabb.getMax();
    
    // Clamp light position to AABB bounds to get closest point
    Vector3 closestPoint;
    closestPoint.X = std::max(aabbMin.X, std::min(lightPos.X, aabbMax.X));
    closestPoint.Y = std::max(aabbMin.Y, std::min(lightPos.Y, aabbMax.Y));
    closestPoint.Z = std::max(aabbMin.Z, std::min(lightPos.Z, aabbMax.Z));
    return (closestPoint - lightPos).Length();
}

std::array<Frustrum, 6> PointLightCuller::createCubemapFrustums(const Vector3& lightPos, float lightRadius) const
{
    std::array<Frustrum, 6> frustums;
    
    for (uint32 i = 0; i < 6; ++i)
    {
        frustums[i] = createFaceFrustum(lightPos, lightRadius, i);
    }
    
    return frustums;
}

Frustrum PointLightCuller::createFaceFrustum(const Vector3& lightPos, float lightRadius, uint32 faceIndex) const
{
    Assert::throwIfFalse(faceIndex < 6, "Face index must be between 0-5");

    // Create 90-degree perspective projection
    float nearPlane = POINT_LIGHT_NEAR_PLANE;
    float farPlane = lightRadius * _settings.faceCullingExpansion; // Slight expansion to avoid edge artifacts
    float aspectRatio = 1.0f; // Square cubemap faces
    
    Matrix4 projection = Matrix4::Perspective(
        Degree(POINT_LIGHT_FOV_DEGREES), 
        aspectRatio, 
        nearPlane, 
        farPlane
    );
    
    // Create view matrix for this face
    Vector3 target = lightPos + FACE_DIRECTIONS[faceIndex];
    Matrix4 view = Matrix4::LookAt(lightPos, target, FACE_UPS[faceIndex]);
    
    // Combine into view-projection matrix
    Matrix4 viewProjection = projection * view;
    
    // Extract frustum from view-projection matrix
    return extractFrustumFromMatrix(viewProjection);
}

bool PointLightCuller::shouldCastShadowForLight(
    const std::shared_ptr<DrawableComponent>& drawable,
    const Vector3& lightPos,
    float lightRadius) const
{
    // Check basic shadow casting eligibility (material, size, etc.)
    auto material = drawable->getMaterial();
    if (!material)
    {
        return false;
    }
    
    // Skip fully transparent objects (could be made more sophisticated)
    if (material->hasOpacityTexture())
    {
        // For now, still render transparent objects in shadow pass
        // Could add opacity threshold checking here
    }
    
    // Check object size
    const Aabb& aabb = drawable->getAabb();
    Vector3 size = aabb.getExtents() * 2.0f;
    
    if (size.X <= 0.0f || size.Y <= 0.0f || size.Z <= 0.0f)
    {
        return false; // Invalid AABB
    }
    
    // Check minimum size threshold
    if (size.X < _settings.minObjectSize && 
        size.Y < _settings.minObjectSize && 
        size.Z < _settings.minObjectSize)
    {
        return false; // Too small to cast meaningful shadows
    }
    
    // Distance-based LOD culling
    if (_settings.enableDistanceLOD)
    {
        float distance = getObjectDistanceFromLight(lightPos, aabb);
        if (distance > _settings.maxShadowDistance)
        {
            return false; // Too far for shadow casting
        }
    }
    
    return true;
}

Frustrum PointLightCuller::extractFrustumFromMatrix(const Matrix4& viewProjection) const
{
    // Extract frustum planes from view-projection matrix
    // This is the same logic as used in ShadowFrustum::extractFrustumFromMatrix
    
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
    
    // Left plane: row4 + row1
    Plane left = createPlaneFromCoeffs(
        viewProjection[3][0] + viewProjection[0][0],
        viewProjection[3][1] + viewProjection[0][1],
        viewProjection[3][2] + viewProjection[0][2],
        viewProjection[3][3] + viewProjection[0][3]
    );
    
    // Right plane: row4 - row1
    Plane right = createPlaneFromCoeffs(
        viewProjection[3][0] - viewProjection[0][0],
        viewProjection[3][1] - viewProjection[0][1],
        viewProjection[3][2] - viewProjection[0][2],
        viewProjection[3][3] - viewProjection[0][3]
    );
    
    // Bottom plane: row4 + row2
    Plane bottom = createPlaneFromCoeffs(
        viewProjection[3][0] + viewProjection[1][0],
        viewProjection[3][1] + viewProjection[1][1],
        viewProjection[3][2] + viewProjection[1][2],
        viewProjection[3][3] + viewProjection[1][3]
    );
    
    // Top plane: row4 - row2
    Plane top = createPlaneFromCoeffs(
        viewProjection[3][0] - viewProjection[1][0],
        viewProjection[3][1] - viewProjection[1][1],
        viewProjection[3][2] - viewProjection[1][2],
        viewProjection[3][3] - viewProjection[1][3]
    );
    
    // Near plane: row4 + row3
    Plane near = createPlaneFromCoeffs(
        viewProjection[3][0] + viewProjection[2][0],
        viewProjection[3][1] + viewProjection[2][1],
        viewProjection[3][2] + viewProjection[2][2],
        viewProjection[3][3] + viewProjection[2][3]
    );
    
    // Far plane: row4 - row3
    Plane far = createPlaneFromCoeffs(
        viewProjection[3][0] - viewProjection[2][0],
        viewProjection[3][1] - viewProjection[2][1],
        viewProjection[3][2] - viewProjection[2][2],
        viewProjection[3][3] - viewProjection[2][3]
    );
    
    return Frustrum(left, right, top, bottom, near, far);
}
